#pragma once

#include <iostream>

#define STARTING_WINDOW_WIDTH 800
#define STARTING_WINDOW_HEIGHT 600
#define GRAVITY (9.8f * 100.f)

#define LEVEL_MAP_GRID_SIZE 16
#define GAME_GRID_SIZE 40

#define AUDIO_FOLDER "audio"
#define LEVELS_FOLDER "levels\\platformer_game\\simplified"
#define ART_FOLDER "art"
#define FONTS_FOLDER "fonts"
#define SHADERS_FOLDER "shaders"

#define BACKGROUND_MUSIC "eden_electro.wav"
#define JUMP_SOUND_EFFECT "jump.wav"
#define STOMP_SOUND_EFFECT "stomp.wav"
#define LEVEL_FINISH_SOUND_EFFECT "level_finish.wav"

#define MAIN_MENU_LEVEL 0
#define GAME_OVER_SCREEN_LEVEL 255

#define game_assert(exp) if ((exp) == false) std::cout << "on line " << __LINE__ << " in file " << __FILE__ << " the game_assert failed" << std::endl;

#define _TESTING 0