cmake_minimum_required(VERSION 3.22.1)
project(stb_image)

add_library(${PROJECT_NAME} 
    stb_image.cpp
    stb/stb_image.h
)