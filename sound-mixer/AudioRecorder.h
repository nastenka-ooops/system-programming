#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H
#include <atomic>
#include <windows.h>
#include <fstream>
#include <vector>

class AudioRecorder {

    HWAVEIN waveIn;
    WAVEFORMATEX waveFormat;
    std::vector<WAVEHDR> waveHeaders;
    uint8_t *buffer;

    uint32_t blockSize;
    uint8_t blockCount;

    std::ofstream outputFile;
    std::atomic<uint32_t> totalDataWritten;

    friend void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

public:
    volatile bool recording;

    AudioRecorder(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels, uint8_t nblockCount, uint32_t nchunkSize);
    ~AudioRecorder();
    bool startRecording(const wchar_t *filename);
    void stopRecording();
};

#endif //AUDIORECORDER_H

