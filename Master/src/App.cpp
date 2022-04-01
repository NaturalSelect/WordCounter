#include <cstdio>
#include <sharpen/EventEngine.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/AsyncMutex.hpp>
#include <sharpen/AsyncSemaphore.hpp>
#include <sharpen/Optional.hpp>
#include <sharpen/ITimer.hpp>
#include <sharpen/AwaitOps.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/CtrlHandler.hpp>
#include <sharpen/Converter.hpp>
#include <wc/MasterTask.hpp>

void HandleClient(sharpen::NetStreamChannelPtr conn,wc::MasterTask *task,sharpen::NetStreamChannelPtr server)
{
    assert(task != nullptr);
    sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
    sharpen::Int32 r = task->CostumeChannel(conn,timer);
    while(!r)
    {
        r = task->CostumeChannel(conn,timer);
    }
    server->Close();
}

void Entry(std::uint16_t port,std::vector<std::string> files)
{
    sharpen::StartupNetSupport();
    sharpen::NetStreamChannelPtr channel = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint ep{0,port};
    channel->Bind(ep);
    channel->Register(sharpen::EventEngine::GetEngine());
    channel->Listen(65535);
    wc::MasterTask task{files.begin(),files.end()};
    std::puts("start map reduce master");
    while (1)
    {
        sharpen::NetStreamChannelPtr conn = channel->AcceptAsync();
        conn->Register(sharpen::EventEngine::GetEngine());
        conn->GetRemoteEndPoint(ep);
        char ip[21] = {};
        ep.GetAddrString(ip,sizeof(ip));
        std::printf("new connection[ip = %s,port = %hu]\n",ip,ep.GetPort());
        sharpen::Launch(&HandleClient,conn,&task,channel);
    }
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        std::puts("usage: <port> <file1> <file2> ... <fileN>");
        return 0;
    }
    sharpen::EventEngine &eng = sharpen::EventEngine::SetupEngine();
    std::vector<std::string> files;
    sharpen::Uint16 port{0};
    port = sharpen::Atoi<sharpen::Uint16>(argv[1],std::strlen(argv[1]));
    for (std::size_t i = 2; i != argc; ++i)
    {
        files.emplace_back(argv[i]);
    }
    eng.Startup(&Entry,port,files);
    return 0;
}