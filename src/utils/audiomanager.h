#pragma once

struct ma_engine;
struct ma_sound;


//helper class for the miniaudio header/library
class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    void playSound(const char* filename);
    void playSoundWithPitch(const char* filename, float pitch);
    void playLoopingSound(const char* filename);
    
private:
    ma_engine* m_engine;
    ma_sound* m_ambientSound;

};
