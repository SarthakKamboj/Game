cmake_minimum_required(VERSION 3.22.1)
project(game)

add_executable(${PROJECT_NAME} 
    main.cpp
)

add_subdirectory(external/glm)

target_include_directories(${PROJECT_NAME} 
    PUBLIC external/glm
    PUBLIC external/SDL2-x86/include/SDL2
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC external/glm
    PUBLIC external/SDL2-x86/lib
)

target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2)