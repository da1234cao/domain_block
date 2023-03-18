cmake_minimum_required (VERSION 3.8)

project ("test")

# link_directories(${PROJECT_SOURCE_DIR}/bin)

add_executable("test_driver" test_driver.cpp)

target_link_libraries("test_driver" block ws2_32.lib)

add_executable("test_service" test_service.cpp)

target_link_libraries("test_service" block ws2_32.lib
                                     spdlog::spdlog
                                     nlohmann_json::nlohmann_json
                                     ${Boost_LIBRARIES})