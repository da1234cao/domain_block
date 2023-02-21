cmake_minimum_required (VERSION 3.8)

project ("block")

add_definitions(-DLIBBLOCK_EXPORTS)

add_library(${PROJECT_NAME} SHARED libblock.cpp)

# All user-mode functions are exported from fwpuclnt.dll.
# All kernel-mode functions are exported from fwpkclnt.sys.
target_link_libraries(${PROJECT_NAME} Fwpuclnt.lib)