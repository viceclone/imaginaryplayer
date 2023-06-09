cmake_minimum_required(VERSION 3.1...3.25)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(
    implayer
    VERSION 0.0
    LANGUAGES CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRC_FILES 
    src/core/playlist.cpp 
    src/core/track.cpp
    src/core/helper.cpp

    src/ui/text_based_player.cpp
)

set(HEADER_FILES 
    include/core/enums.hpp
    include/core/helper.hpp
    include/core/playlist.hpp 
    include/core/track.hpp
    include/core/logger.hpp
    include/core/constants.hpp

    include/ui/iplayer.hpp
    include/ui/text_based_player.hpp
)

add_library(${PROJECT_NAME}_lib ${SRC_FILES} ${HEADER_FILES})

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)