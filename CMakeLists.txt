cmake_minimum_required(VERSION 3.1...3.25)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(
    implayer
    VERSION 0.0
    LANGUAGES CXX)

set(SRC_FILES 
    src/player.cpp 
    src/playlist.cpp 
    src/track.cpp
)

set(HEADER_FILES 
    include/player.hpp 
    include/playlist.hpp 
    include/track.hpp
)

add_library(${PROJECT_NAME}_lib ${SRC_FILES} ${HEADER_FILES})

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)