//
// Created by madam on 06.12.2024.
//

#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include "AudioBufffer.h"

class AudioSource {
    AudioBuffer* buffer;
    uint32_t position;

    float volume;
    float pan;
    float speed;

    uint8_t status;
    bool loop;

    friend class AudioMixer;

    AudioSource(AudioBuffer* buffer = nullptr);

    ~AudioSource();

public:
    enum {
        STOP,
        PLAY,
        PAUSE,
    };

    void setBuffer(AudioBuffer* buffer);

    AudioBuffer* getBuffer() const;

    bool play();

    bool pause();

    bool stop();

    bool finished();

    uint8_t getStatus() const;

    double getElapsedSeconds() const;

    double getTotalSeconds() const;

    uint32_t getSampleCount() const;

    void setPosition(uint32_t value);

    uint32_t getPosition() const;

    void setProgress(float value);

    float getProgress() const;

    void setLoop(bool value);

    bool getLoop() const;

    /*
    Value: 0.0 -> 1.0
    Default: 1.0
    */
    void setVolume(float value);

    float getVolume() const;

    /*
    Values: 0.0 -> 1.0
    Default: 0.5
    */
    void setPan(float value);

    float getPan() const;

    void setSpeed(float value);

    float getSpeed() const;
};

#endif //AUDIOSOURCE_H
