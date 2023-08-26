cmake_minimum_required(VERSION 3.22.1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

project(game)

message(CMAKE_BUILD_TYPE)
message(${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/networking.cpp
    src/input/input.cpp
    src/init.cpp
    src/renderer/renderer.cpp
)

add_subdirectory(external/enet)
add_subdirectory(external/glad)

target_include_directories(${PROJECT_NAME} 
    PUBLIC external/enet/include
    PUBLIC src
    PUBLIC ../shared
    PUBLIC external/SDL2_x64/include/SDL2
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC external/enet
    PUBLIC external/SDL2_x64/lib
)

target_link_libraries(${PROJECT_NAME} stdc++)
target_link_libraries(${PROJECT_NAME} enet)
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2)