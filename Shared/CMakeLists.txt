cmake_minimum_required(VERSION 3.14.0)

set(SHARPEN_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/lib/include")

file(GLOB_RECURSE src "${PROJECT_SOURCE_DIR}/Shared/src" "*.h" "*.hpp" "*.cpp" "*.cc")

file(GLOB_RECURSE SHARED_HEADERS "${PROJECT_SOURCE_DIR}/include" "*.h" "*.hpp")

include_directories("${SHARPEN_INCLUDE_DIR}" "${PROJECT_SOURCE_DIR}/Shared/include")

add_library(PwordsShared ${src} ${SHARED_HEADERS})

target_link_libraries(PwordsShared sharpen::sharpen)