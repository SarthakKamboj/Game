# cmake_minimum_required(VERSION 3.22.1)

# if(NOT CMAKE_BUILD_TYPE)
#   set(CMAKE_BUILD_TYPE "Debug")
# endif()

# project(game)

# message(CMAKE_BUILD_TYPE)
# message(${CMAKE_BUILD_TYPE})

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# if (TESTING)
    
#     if (TESTCASES)
#         add_executable(${PROJECT_NAME}
#             src/tests/test.cpp 
#             src/tests/test2.cpp 

#             src/app.cpp
#             src/networking.cpp
#             src/input/input.cpp
#             src/init.cpp
#             src/renderer/renderer.cpp
#             src/utils/time.cpp
#             src/utils/io.cpp
#             src/renderer/basic/shape_renders.cpp
#             src/renderer/opengl/buffers.cpp
#             src/renderer/opengl/object_data.cpp
#             src/renderer/opengl/resources.cpp
#             src/renderer/opengl/vertex.cpp
#             src/transform/transform.cpp
#         )

#         add_subdirectory(external/Catch2)
#         target_link_libraries(${PROJECT_NAME} Catch2::Catch2WithMain)
#     else()
#         add_executable(${PROJECT_NAME} 
#             src/main.cpp
#             src/app.cpp
#             src/input/input.cpp
#             src/init.cpp
#             src/renderer/renderer.cpp
#             src/utils/time.cpp
#             src/utils/io.cpp
#             src/renderer/basic/shape_renders.cpp
#             src/renderer/opengl/buffers.cpp
#             src/renderer/opengl/object_data.cpp
#             src/renderer/opengl/resources.cpp
#             src/renderer/opengl/vertex.cpp
#             src/transform/transform.cpp

#             src/networking/networking.mocked.cpp
#         )
#     endif()

#     add_subdirectory(external/enet)
#     add_subdirectory(external/glad)
#     add_subdirectory(external/stb_image)

#     target_link_directories(${PROJECT_NAME} 
#         PUBLIC external/SDL2_x64
#     )

#     target_link_libraries(${PROJECT_NAME} enet)
#     target_link_libraries(${PROJECT_NAME} glad)
#     target_link_libraries(${PROJECT_NAME} stb_image)
#     target_link_libraries(${PROJECT_NAME} SDL2main SDL2)

#     target_include_directories(${PROJECT_NAME} 
#         PUBLIC ../shared
#         PUBLIC src
#         PUBLIC external/Catch2/src
#         PUBLIC external/enet/include
#         PUBLIC external/SDL2_x64/include/SDL2
#         PUBLIC external/glad/include
#         PUBLIC external/glm
#         PUBLIC external/stb_image
#     )

# else()

# add_executable(${PROJECT_NAME} 
#     src/main.cpp
#     src/app.cpp
#     src/update.cpp
#     src/networking/networking.cpp
#     src/input/input.cpp
#     src/init.cpp
#     src/renderer/renderer.cpp
#     src/utils/time.cpp
#     src/utils/io.cpp
#     src/renderer/basic/shape_renders.cpp
#     src/renderer/opengl/buffers.cpp
#     src/renderer/opengl/object_data.cpp
#     src/renderer/opengl/resources.cpp
#     src/renderer/opengl/vertex.cpp
#     src/transform/transform.cpp
# )

# add_subdirectory(external/enet)
# add_subdirectory(external/glad)
# add_subdirectory(external/stb_image)
# add_subdirectory(../shared external/shared)

# target_include_directories(${PROJECT_NAME} 
#     PUBLIC ../shared
#     PUBLIC src
#     PUBLIC external/enet/include
#     PUBLIC external/SDL2_x64/include/SDL2
#     PUBLIC external/glad/include
#     PUBLIC external/glm
#     PUBLIC external/stb_image
# )

# target_link_directories(${PROJECT_NAME} 
#     PUBLIC external/SDL2_x64
# )

# # target_link_libraries(${PROJECT_NAME} stdc++)
# # target_link_libraries(${PROJECT_NAME} winmm ws2_32)
# # target_link_libraries(${PROJECT_NAME} winmm ws2_32 enet)

# # target_link_libraries(${PROJECT_NAME} enet64)
# target_link_libraries(${PROJECT_NAME} enet)
# target_link_libraries(${PROJECT_NAME} glad)
# target_link_libraries(${PROJECT_NAME} stb_image)
# target_link_libraries(${PROJECT_NAME} shared)
# target_link_libraries(${PROJECT_NAME} SDL2main SDL2)
# # target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2)

# endif()


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

if (RUN_TESTCASES)
    message("with test cases")
    configure_test_cases()
else()
    message("without test cases")
    configure_game()
endif() 

# if (RUN_MOCKED) 
#     add_executable()
# else()

# endif()

# add_subdirectory(external/enet)
# add_subdirectory(external/glad)
# add_subdirectory(external/stb_image)
# add_subdirectory(../shared external/shared)

# target_include_directories(${PROJECT_NAME} 
#     PUBLIC ../shared
#     PUBLIC src
#     PUBLIC external/enet/include
#     PUBLIC external/SDL2_x64/include/SDL2
#     PUBLIC external/glad/include
#     PUBLIC external/glm
#     PUBLIC external/stb_image
# )

# target_link_directories(${PROJECT_NAME} 
#     PUBLIC external/SDL2_x64
# )

# # target_link_libraries(${PROJECT_NAME} stdc++)
# # target_link_libraries(${PROJECT_NAME} winmm ws2_32)
# # target_link_libraries(${PROJECT_NAME} winmm ws2_32 enet)

# # target_link_libraries(${PROJECT_NAME} enet64)
# target_link_libraries(${PROJECT_NAME} enet)
# target_link_libraries(${PROJECT_NAME} glad)
# target_link_libraries(${PROJECT_NAME} stb_image)
# target_link_libraries(${PROJECT_NAME} shared)
# target_link_libraries(${PROJECT_NAME} SDL2main SDL2)
# # target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2)

# endif()