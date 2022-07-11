cmake_minimum_required(VERSION 3.0)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(thread_demo thread_demo.cpp)
target_link_libraries(thread_demo Qt5::Core ${CMAKE_THREAD_LIBS_INIT})
