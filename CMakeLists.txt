cmake_minimum_required(VERSION 3.14.0)
project(PWords VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED on)

include(CTest)
enable_testing()

include(FetchContent)

fetchcontent_declare(sharpen GIT_REPOSITORY "http://github.com/KnownSpace/Sharpen" GIT_TAG "1.0.2-RC-1" SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib")
fetchcontent_makeavailable(sharpen)

add_subdirectory("${PROJECT_SOURCE_DIR}/Shared")
add_subdirectory("${PROJECT_SOURCE_DIR}/Master")
add_subdirectory("${PROJECT_SOURCE_DIR}/Worker")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
