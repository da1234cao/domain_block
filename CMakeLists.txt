cmake_minimum_required (VERSION 3.11)

project ("doamin_block")

# 可执行文件输出到bin目录
## 静态库
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
## 动态库和可执行文件
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# utis和server
include_directories(${CMAKE_SOURCE_DIR})

# boost
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  find_package(Boost REQUIRED COMPONENTS thread system filesystem)
else()
  find_package(Boost REQUIRED)
endif()
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif()
message(STATUS "Boost version: ${Boost_VERSION}")
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include(FetchContent)

#spdlog-但是这就仓库的拉取还是比较耗时
## github访问较慢，使用gitee代替
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://gitee.com/moondaisy/spdlog
        GIT_TAG        v1.x
)
FetchContent_MakeAvailable(spdlog)
include_directories(${spdlog_SOURCE_DIR}/include)
message("fetch spdlog from gitee instead of github")

# toml: 
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://gitee.com/actcwlf/tomlplusplus.git
    GIT_TAG        v3.1.0
)
FetchContent_MakeAvailable(tomlplusplus)
include_directories(${tomlplusplus_SOURCE_DIR}/include)
message("fetch toml from gitee instead of github")

# json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)
include_directories(${tomlplusplus_SOURCE_DIR}/include)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    if(MSVC)
      # Disable warning C4819 (cannot represent character) for all targets
      add_compile_options(/wd4819) # 字符提示禁用.代码中有的是在linux中编辑的是utf8编码，win上默认是gbk编码
      add_compile_options(/wd4101) # 禁用未使用局部变量提示。
      # vs库静态链接
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" /MT)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE}" /MTd)
      set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" /MT)
      set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE}" /MTd)
    endif()
    add_subdirectory(windows/WFP)
else()
    add_subdirectory(UI/domain_block_client)
    add_subdirectory(linux)
endif()