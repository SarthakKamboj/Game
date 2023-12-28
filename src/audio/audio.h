#pragma once

#include "AL/al.h"
#include "AL/alc.h"

struct read_status {
    bool success = false;
    ALuint al_handle = 0;
};

struct sound_device_t {
    ALCdevice* device;
    ALCcontext* context;
};

struct audio_source_t {
    ALuint al_handle;
};

struct sound_t {
    ALuint al_buffer_handle;
    int sample_rate;
    int num_channels;
};

void init_audio();
bool detect_error();

void read_wav_sound(const char* name, const char *filename, bool bck_sound = false);
audio_source_t create_audio_source();
void play_bck_sound();
void play_sound(const char* sound_name);