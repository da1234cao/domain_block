cmake_minimum_required (VERSION 3.8)

project ("test")

# link_directories(${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} block ws2_32.lib)