cmake_minimum_required(VERSION 3.22.1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

project(game)

message(${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RUN_MOCKED "Running in mocked network environment" ON)
option(RUN_TESTCASES "Run unit tests and test cases" OFF)

function(configure_test_cases)
    add_subdirectory(external/Catch2)

    add_executable(${PROJECT_NAME} 
        src/tests/test.cpp
    )

    target_include_directories(${PROJECT_NAME} 
        PUBLIC external/Catch2/src
        PUBLIC src
    )

    target_link_libraries(${PROJECT_NAME} Catch2::Catch2WithMain)

endfunction()

function(configure_game)

    set(EXTRA_FILES)

    if (RUN_MOCKED)
        message("mocked")
        list(APPEND EXTRA_FILES 
            src/main.mocked.cpp
            src/networking/networking.mocked.cpp
        )
    else()
        message("unmocked")
        
        list(APPEND EXTRA_FILES 
            src/main.real.cpp
            src/networking/networking.real.cpp
        )
    endif()

    add_executable(${PROJECT_NAME} 
        src/app.cpp
        src/update.cpp
        src/networking/networking.cpp
        src/input/input.cpp
        src/init.cpp
        src/renderer/renderer.cpp
        src/utils/time.cpp
        src/utils/io.cpp
        src/renderer/basic/shape_renders.cpp
        src/renderer/opengl/buffers.cpp
        src/renderer/opengl/object_data.cpp
        src/renderer/opengl/resources.cpp
        src/renderer/opengl/vertex.cpp
        src/transform/transform.cpp

        ${EXTRA_FILES}
    )

    add_subdirectory(external/enet)
    add_subdirectory(external/glad)
    add_subdirectory(external/stb_image)
    add_subdirectory(../shared external/shared)

    target_include_directories(${PROJECT_NAME} 
        PUBLIC ../shared
        PUBLIC src
        PUBLIC external/SDL2_x64/include/SDL2
        PUBLIC external/glad/include
        PUBLIC external/glm
        PUBLIC external/stb_image
        PUBLIC external/enet/include
    )

    target_link_directories(${PROJECT_NAME} 
        PUBLIC external/SDL2_x64
    )

    target_link_libraries(${PROJECT_NAME} glad)
    target_link_libraries(${PROJECT_NAME} stb_image)
    target_link_libraries(${PROJECT_NAME} shared)
    target_link_libraries(${PROJECT_NAME} SDL2main SDL2)
    target_link_libraries(${PROJECT_NAME} enet)

endfunction()

configure_file(test_config.h.in ${CMAKE_SOURCE_DIR}/src/test_config.h)
if (RUN_TESTCASES)
    message("with test cases")
    configure_test_cases()
else()
    message("without test cases")
    configure_game()
endif() 