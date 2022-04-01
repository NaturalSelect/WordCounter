cmake_minimum_required(VERSION 3.14.0)

set(SHARPEN_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/lib/include")

file(GLOB_RECURSE src "${PROJECT_SOURCE_DIR}/Worker/src" "*.h" "*.hpp" "*.cpp" "*.cc")

set(SHARED_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Shared/include")

include_directories("${SHARPEN_INCLUDE_DIR}" "${SHARED_INCLUDE_DIR}" "${PROJECT_SOURCE_DIR}/Worker/include")

add_executable(Worker ${src})

target_link_libraries(Worker sharpen::sharpen PwordsShared)