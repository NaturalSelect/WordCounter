#include <cstdio>
#include <map>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/Optional.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/Converter.hpp>
#include <wc/OkMessage.hpp>
#include <wc/ReduceMessage.hpp>
#include <wc/MapMessage.hpp>
#include <wc/PingMessage.hpp>

const char tagTable[] = "abcdefghijklmnopqrstuvwxyz";

sharpen::Optional<sharpen::ByteBuffer> GetChannelMessage(sharpen::NetStreamChannelPtr channel)
{
    wc::MessageHeader header;
    char *p = reinterpret_cast<char*>(&header);
    for (auto begin = p,end = p + sizeof(header); begin != end;)
    {
        std::size_t rz = channel->ReadAsync(begin,end - begin);
        begin += rz;
        if(!rz)
        {
            return sharpen::EmptyOpt;
        }   
    }
    sharpen::ByteBuffer buf{header.size_};
    std::memcpy(buf.Data(),&header,sizeof(header));
    p = buf.Data() + sizeof(header);
    for (auto begin = p,end = buf.Data() + buf.GetSize(); begin != end;)
    {
        std::size_t rz = channel->ReadAsync(begin,end - begin);
        begin += rz;
        if(!rz)
        {
            return sharpen::EmptyOpt;
        }
    }
    return buf;
}

std::string Map(const std::string &fileName)
{
    std::string resultName{fileName};
    resultName += ".im";
    std::printf("mapping %s to %s\n",fileName.c_str(),resultName.c_str());
    sharpen::FileChannelPtr channel = sharpen::MakeFileChannel(resultName.c_str(),sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
    channel->Register(sharpen::EventEngine::GetEngine());
    sharpen::FileChannelPtr input = sharpen::MakeFileChannel(fileName.c_str(),sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
    input->Register(sharpen::EventEngine::GetEngine());
    sharpen::ByteBuffer buf{4096};
    sharpen::Uint64 size{input->GetFileSize()};
    std::string word;
    std::map<std::string,sharpen::Uint64> map;
    for (sharpen::Uint64 i = 0; i != size;)
    {
        sharpen::Size rz = input->ReadAsync(buf,i);
        for (sharpen::Size j = 0; j != rz; ++j)
        {
            if (buf[j] == ' ' || buf[j] == '\n' || buf[j] == '\r')
            {
                if(!word.empty())
                {
                    auto ite = map.find(word);
                    if(ite != map.end())
                    {
                        ite->second += 1;
                    }
                    else
                    {
                        std::string tmp;
                        std::swap(tmp,word);
                        map.emplace(std::move(tmp),1);
                    }
                    word.clear();
                }   
                continue;
            }
            word.push_back(buf[j]);
        }
        i += rz;   
    }
    sharpen::Uint64 off{0};
    for (auto begin = map.begin(),end = map.end(); begin != end; ++begin)
    {
        std::string str{begin->first};
        str += ' ';
        str += std::to_string(begin->second);
        str += '\n';
        channel->WriteAsync(str.data(),str.size(),off);
        off += str.size();
    }
    return resultName;
}

std::string Reduce(sharpen::Uint32 tag,const std::vector<std::string> &files)
{
    std::string resultName{std::to_string(tag)};
    resultName += "_result.txt";
    sharpen::FileChannelPtr out = sharpen::MakeFileChannel(resultName.c_str(),sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
    out->Register(sharpen::EventEngine::GetEngine());
    std::map<std::string,sharpen::Uint64> map;
    for (auto begin = files.begin(),end = files.end(); begin != end; ++begin)
    {
        sharpen::FileChannelPtr channel = sharpen::MakeFileChannel(begin->c_str(),sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
        channel->Register(sharpen::EventEngine::GetEngine());
        std::string line;
        sharpen::ByteBuffer buf{4096};
        sharpen::Uint64 size{channel->GetFileSize()};
        for (sharpen::Uint64 i = 0; i != size;)
        {
            sharpen::Size rz = channel->ReadAsync(buf,i);
            for (sharpen::Size j = 0; j != rz; ++j)
            {
                if(buf[j] == '\n' || buf[j] == '\r')
                {
                    if(!line.empty() && std::tolower(line.front()) == tagTable[tag])
                    {
                        std::puts(line.c_str());
                        sharpen::Size pos = line.find(' ');
                        if(pos == line.npos || pos == 0)
                        {
                            continue;
                        }
                        std::string word{line.begin(),line.begin() + pos - 1};
                        sharpen::Uint64 count{0};
                        count = sharpen::Atoi<sharpen::Uint64>(line.data() + pos + 1,line.size() - pos - 1);
                        auto ite = map.find(word);
                        if(ite != map.end())
                        {
                            ite->second += count;
                        }
                        else
                        {
                            std::string tmp;
                            std::swap(tmp,word);
                            map.emplace(std::move(tmp),count);
                        }
                    }
                    line.clear();
                    continue;
                }
                line.push_back(buf[j]);   
            }
            i += rz;   
        }
    }
    sharpen::Uint64 off{0};
    std::printf("reduce %c size %zu\n",tagTable[tag],map.size());
    for (auto begin = map.begin(),end = map.end(); begin != end; ++begin)
    {
        if (begin->first.empty())
        {
            continue;
        }
        std::string str{begin->first};
        str += '=';
        str += std::to_string(begin->second);
        str += '\n';
        out->WriteAsync(str.data(),str.size(),off);
        off += str.size();
    }
    return resultName;
}

void Entry(const char *ip,std::uint16_t port)
{
    sharpen::StartupNetSupport();
    sharpen::NetStreamChannelPtr channel = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint ep{0,0};
    channel->Bind(ep);
    ep.SetPort(port);
    ep.SetAddrByString(ip);
    channel->Register(sharpen::EventEngine::GetEngine());
    try
    {
        channel->ConnectAsync(ep);
        //wait request
        while (1)
        {
            auto msgBuf{GetChannelMessage(channel)};
            if(!msgBuf.Exist())
            {
                return;
            }
            sharpen::ByteBuffer &buf{msgBuf.Get()};
            wc::MessageHeader *header = reinterpret_cast<wc::MessageHeader*>(buf.Data());
            if (header->type_ == static_cast<std::uint64_t>(wc::MessageType::Map))
            {
                wc::MapMessage msg;
                msg.LoadFrom(buf);
                std::string res;
                auto future = sharpen::Async(&Map,msg.GetFileName());
                sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
                while (future->IsPending())
                {
                    wc::PingMessage pmsg;
                    sharpen::ByteBuffer pbuf;
                    pmsg.StoreTo(pbuf);
                    channel->WriteAsync(pbuf);
                    timer->Await(std::chrono::milliseconds(100));
                }
                res = future->Await();
                wc::OkMessage rep{res};
                sharpen::ByteBuffer resBuf;
                rep.StoreTo(resBuf);
                channel->WriteAsync(resBuf);
            }
            else
            {
                wc::ReduceMessage msg{0};
                msg.LoadFrom(buf);
                std::printf("reduce tag %c\n",tagTable[msg.GetTag()]);
                std::string res;
                std::vector<std::string> files{msg.Begin(),msg.End()};
                auto future = sharpen::Async(&Reduce,msg.GetTag(),files);
                sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
                while (future->IsPending())
                {
                    wc::PingMessage pmsg;
                    sharpen::ByteBuffer pbuf;
                    pmsg.StoreTo(pbuf);
                    channel->WriteAsync(pbuf);
                    timer->Await(std::chrono::milliseconds(100));
                }
                res = future->Await();
                wc::OkMessage rep{res};
                sharpen::ByteBuffer resBuf;
                rep.StoreTo(resBuf);
                channel->WriteAsync(resBuf);
            }
        }
    }
    catch(const std::exception &e)
    {
        std::fprintf(stderr,"error[msg = %s]\n",e.what());
    }
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        std::puts("usage: <ip> <port> - connect to ip:port");
        return 0;
    }
    sharpen::EventEngine &eng = sharpen::EventEngine::SetupSingleThreadEngine();
    sharpen::Uint16 port{0};
    port = sharpen::Atoi<sharpen::Uint16>(argv[2],std::strlen(argv[2]));
    eng.Startup(&Entry,argv[1],8080);
    return 0;
}