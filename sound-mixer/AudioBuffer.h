#ifndef AUDIOBUFFFER_H
#define AUDIOBUFFFER_H

#include <cstdint>
#include <cstdio>

class AudioBuffer {
    uint8_t* data;
    uint32_t length;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
    uint16_t channelCount;

public:
    AudioBuffer();

    ~AudioBuffer();

    uint8_t* create(uint32_t length, uint32_t sampleRate = 44100, uint16_t bitsPerSample = 16, uint16_t channelCount = 1);

    bool load(const wchar_t* filename);

    FILE* startSave(const wchar_t* filename);
    bool stopSave(FILE* outputFile);

    void clear();

    uint32_t getSampleRate() const;

    uint16_t getBitsPerSample() const;

    uint16_t getChannelCount() const;

    uint32_t getLength() const;

    uint8_t* getData() const;
};

#endif // AUDIOBUFFFER_H
