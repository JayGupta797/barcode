cmake_minimum_required(VERSION 3.30.0)
project(barcode)
set(CMAKE_CXX_STANDARD 20)
find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)