cmake_minimum_required(VERSION 3.22.1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

project(game)

message(${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(configure_game)

    set(FILES)
    list(APPEND FILES 
        src/app.cpp
        src/camera.cpp
        src/update.cpp
        src/physics/physics.cpp
        src/input/input.cpp
        src/init.cpp
        src/renderer/renderer.cpp
        src/utils/time.cpp
        src/utils/math.cpp
        src/utils/io.cpp
        src/renderer/basic/shape_renders.cpp
        src/renderer/opengl/buffers.cpp
        src/renderer/opengl/object_data.cpp
        src/renderer/opengl/resources.cpp
        src/renderer/opengl/vertex.cpp
        src/transform/transform.cpp
        src/gameobjects/gos.cpp
        src/animation/animation.cpp
        src/ui/ui.cpp
        src/audio/audio.cpp
        src/main.cpp

        resources/ico.rc
    )

    add_executable(${PROJECT_NAME} 
        WIN32
        ${FILES}
    )

    add_subdirectory(external/glad)
    add_subdirectory(external/stb_image)
    add_subdirectory(external/ft2132/freetype-2.13.2)
    add_subdirectory(external/openal-soft)
    add_subdirectory(external/libsndfile)

    target_include_directories(${PROJECT_NAME} 
        PUBLIC src

        PUBLIC external/SDL2_x64/include/SDL2
        PUBLIC external/glad/include
        PUBLIC external/glm
        PUBLIC external/stb_image
        PUBLIC external/ft2132/freetype-2.13.2/include
        PUBLIC external/openal-soft/include
        PUBLIC external/libsndfile/include
    )

    target_link_directories(${PROJECT_NAME} 
        PUBLIC external/SDL2_x64
        PUBLIC out/build/external/openal-soft/Debug
    )

    target_link_libraries(${PROJECT_NAME} glad)
    target_link_libraries(${PROJECT_NAME} stb_image)
    target_link_libraries(${PROJECT_NAME} freetype)
    target_link_libraries(${PROJECT_NAME} SDL2main SDL2)
    target_link_libraries(${PROJECT_NAME} OpenAL)
    target_link_libraries(${PROJECT_NAME} sndfile)

endfunction()


configure_game()
