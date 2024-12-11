#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <windows.h>

#include <cstdint>
#include <fstream>
#include <pthread.h>
#include <vector>

#include "AudioSource.h"
#include "AudioBuffer.h"

#define AUDIO_MIXER_MAX_SOURCE_COUNT 16

class AudioMixer {

    HWAVEOUT waveOut;
    WAVEFORMATEX waveFormat;
    WAVEHDR* waveHeaders;
    friend void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
    friend void* AudioThreadProc(void*);

    uint8_t* waveHeaderBuffer;
    int32_t* accumulator;

    volatile uint8_t freeBlocks;
    uint32_t blockCount;
    uint32_t blockSize;
    uint32_t blockIndex;

    std::vector<AudioSource*> sources;

    uint32_t samplesPerSec;
    uint16_t bitsPerSample;
    uint16_t channelCount;

    pthread_mutex_t freeBlocksMutex;
    pthread_cond_t freeBlocksCond;
    pthread_t audioThread;
    volatile bool ready;

    std::ofstream outputFile;
    uint32_t totalDataWritten;

    bool create(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels, uint8_t nblockCount, uint32_t nchunkSize);
    void destroy();
public:
    AudioMixer(uint32_t sampleRate = 44100, uint16_t bitsPerSample = 16, uint16_t channels = 2, uint8_t nblockCount = 4, uint32_t nchunkSize = 1024);

    ~AudioMixer();

    std::vector<AudioSource*> getSources();

    void setSources(std::vector<AudioSource*>);

    AudioSource* create(AudioBuffer* audioBuffer, const wchar_t* name);

    AudioSource* create(const wchar_t*);

    AudioSource* play(AudioBuffer* audioBuffer);

    AudioSource* play(const wchar_t* filename);

    bool startRecording(const std::string &filename);
    void stopRecording();
};

#endif // AUDIOMIXER_H
