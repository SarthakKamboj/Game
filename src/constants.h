#pragma once

#include <string>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
// TODO: possibly generalize this in the future
#define RESOURCES_PATH std::string("C:\\Sarthak\\projects\\game\\resources")
#define SHADERS_PATH (RESOURCES_PATH + std::string("\\shaders"))
#define GRAVITY (9.8f * 100.f)
#define MAX_HORIZONTAL_COL_OFFSET_PIXELS 2
#define MAX_PIXELS_OVERLAP_FOR_VERT_COL 3.5f
// #define WORLD_ITEM_TEXT_FILE_DELIM "GAME_DELIM"
#define WORLD_ITEM_TEXT_FILE_DELIM " , "
#define NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION 3
// #define MAX_SNAPSHOT_BUFFER_SIZE (NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION * 2)
#define MAX_SNAPSHOT_BUFFER_SIZE NUM_SNAPSHOTS_FOR_SAFE_INTERPOLATION
// #define MAX_SNAPSHOT_BUFFER_SIZE 4 