cmake_minimum_required (VERSION 3.8)

project ("doamin_block")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" /MT)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE}" /MTd)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" /MT)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE}" /MTd)

# 静态库
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 动态库和可执行文件
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include(FetchContent)

#spdlog
## github访问较慢，使用gitee代替
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://gitee.com/moondaisy/spdlog
        GIT_TAG        v1.x
)
FetchContent_MakeAvailable(spdlog)
include_directories(${spdlog_SOURCE_DIR}/include)
message("fetch spdlog from gitee instead of github")

# 编译驱动接口的库
add_subdirectory(libblock)

# 编译测试程序
add_subdirectory(test)