cmake_minimum_required (VERSION 3.11)

project ("domain_block_service")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# server
set(SERVER_SRC ${CMAKE_SOURCE_DIR}/server/connection.cpp)

# service
set(SERVICE_SRC main.cpp
                config.cpp)

# utils
set(UTILS_SRC ${CMAKE_SOURCE_DIR}/utils/log.cpp)
                
# all src
set(SRC ${SERVICE_SRC} ${SERVER_SRC} ${UTILS_SRC})

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE 
                         tomlplusplus::tomlplusplus 
                         spdlog::spdlog
                         nlohmann_json::nlohmann_json
                         ${Boost_LIBRARIES}
                         dl)