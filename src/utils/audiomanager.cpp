#define MINIAUDIO_IMPLEMENTATION
#include "audiomanager.h"
#include "../miniaudio.h"
#include <QFile>
#include <iostream>

AudioManager::AudioManager() : m_engine(nullptr), m_ambientSound(nullptr) {
    m_engine = new ma_engine;
    ma_result result = ma_engine_init(NULL, m_engine);
    if (result != MA_SUCCESS) {
        delete m_engine;
        m_engine = nullptr;
    }
}

AudioManager::~AudioManager() {
    if (m_ambientSound != nullptr) {
        ma_sound_uninit(m_ambientSound);
        delete m_ambientSound;
        m_ambientSound = nullptr;
    }
    
    if (m_engine != nullptr) {
        ma_engine_uninit(m_engine);
        delete m_engine;
        m_engine = nullptr;
    }
}

void AudioManager::playSound(const char* filename) {
    if (m_engine == nullptr) {
        return;
    }
    
    ma_engine_play_sound(m_engine, filename, NULL);
}

static void soundEndCallback(void* pUserData, ma_sound* pSound) {
    if (pSound) {
        ma_sound_uninit(pSound);
        delete pSound;
    }
}

void AudioManager::playSoundWithPitch(const char* filename, float pitch) {
    if (m_engine == nullptr) {
        return;
    }
    
    ma_sound* sound = new ma_sound;
    ma_result result = ma_sound_init_from_file(m_engine, filename, 
                                                MA_SOUND_FLAG_NO_SPATIALIZATION, 
                                                NULL, NULL, sound);
    if (result != MA_SUCCESS) {
        delete sound;
        return;
    }
    
    ma_sound_set_pitch(sound, pitch);
    ma_sound_set_end_callback(sound, soundEndCallback, NULL);
    
    ma_sound_start(sound);
}


//for background music
void AudioManager::playLoopingSound(const char* filename) {
    if (m_engine == nullptr) {
        return;
    }
    
    QFile file(filename);
    if (!file.exists()) {
        return;
    }
    
    if (m_ambientSound != nullptr) {
        ma_sound_stop(m_ambientSound);
        ma_sound_uninit(m_ambientSound);
        delete m_ambientSound;
        m_ambientSound = nullptr;
    }
    
    m_ambientSound = new ma_sound;
    
    //MA_SOUND_FLAG_STREAM tells miniaudio to stream the file instead of loading it entirely
    ma_result result = ma_sound_init_from_file(m_engine, filename, 
                                                MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_STREAM, 
                                                NULL, NULL, m_ambientSound);
    
    if (result != MA_SUCCESS) {
        delete m_ambientSound;
        m_ambientSound = nullptr;
        return;
    }
    
    ma_sound_set_looping(m_ambientSound, MA_TRUE);
    ma_sound_start(m_ambientSound);
}

