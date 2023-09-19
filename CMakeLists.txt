cmake_minimum_required(VERSION 3.22)

project(bt-tinker)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(bt-tinker 
src/device-handler.cpp
src/device-finder.cpp
main.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth
NO_DEFAULT_PATH
PATHS "~/Qt/6.5.2/gcc_64/lib/cmake")

target_link_libraries(bt-tinker Qt6::Bluetooth Qt6::Core)

target_include_directories(bt-tinker
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include)