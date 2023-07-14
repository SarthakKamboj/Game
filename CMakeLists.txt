cmake_minimum_required(VERSION 3.22.1)
project(game)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/init.cpp
    src/app.cpp
    src/utils/conversion.cpp
    src/utils/io.cpp
    src/utils/time.cpp
    src/transform/transform.cpp
    src/renderer/renderer.cpp
    src/renderer/basic/shape_renders.cpp
    src/renderer/opengl/buffers.cpp
    src/renderer/opengl/object_data.cpp
    src/renderer/opengl/resources.cpp
    src/renderer/opengl/vertex.cpp
    src/physics/physics.cpp
    src/input/input.cpp
    src/gameobjects/gos.cpp
    src/animation/animation.cpp
)

add_subdirectory(external/glm)
add_subdirectory(external/glad)
add_subdirectory(external/stb_image)

target_include_directories(${PROJECT_NAME} 
    PUBLIC external/glm
    PUBLIC external/glad/include
    PUBLIC external/SDL2-x86/include/SDL2
    PUBLIC external/stb_image
    PUBLIC src
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC external/glm
    PUBLIC external/glad
    PUBLIC external/SDL2-x86/lib
    PUBLIC external/stb_image
)

target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} stb_image)
target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2)