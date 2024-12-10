#include "Utils.h"
#include "AudioRecorder.h"

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    AudioRecorder *recorder = reinterpret_cast<AudioRecorder *>(dwInstance);

    switch (uMsg) {
        case WIM_DATA: {
            WAVEHDR *header = reinterpret_cast<WAVEHDR *>(dwParam1);

            if (recorder->recording && recorder->outputFile.is_open()) {
                recorder->outputFile.write(header->lpData, header->dwBytesRecorded);
                recorder->totalDataWritten += header->dwBytesRecorded;
            }

            waveInAddBuffer(hwi, header, sizeof(WAVEHDR));
            break;
        }
    }
}


AudioRecorder::AudioRecorder(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels, uint8_t nblockCount,
                             uint32_t nchunkSize)
    : recording(false), totalDataWritten(0), blockCount(nblockCount), blockSize(nchunkSize) {
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nSamplesPerSec = sampleRate;
    waveFormat.wBitsPerSample = bitsPerSample;
    waveFormat.nChannels = channels;
    waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    waveInOpen(&waveIn, WAVE_MAPPER, &waveFormat, (DWORD_PTR) waveInProc, (DWORD_PTR) this, CALLBACK_FUNCTION);

    buffer = new uint8_t[blockSize * blockCount];
    waveHeaders.resize(blockCount);

    for (uint8_t i = 0; i < blockCount; ++i) {
        WAVEHDR &header = waveHeaders[i];
        header.lpData = reinterpret_cast<LPSTR>(buffer + i * blockSize);
        header.dwBufferLength = blockSize;
        header.dwFlags = 0;
        header.dwLoops = 0;

        waveInPrepareHeader(waveIn, &header, sizeof(WAVEHDR));
        waveInAddBuffer(waveIn, &header, sizeof(WAVEHDR));
    }
}

AudioRecorder::~AudioRecorder() {
    stopRecording();

    for (auto &header: waveHeaders) {
        waveInUnprepareHeader(waveIn, &header, sizeof(WAVEHDR));
    }

    waveInClose(waveIn);
    delete[] buffer;
}

bool AudioRecorder::startRecording(const wchar_t *filename) {
    if (recording) {
        return false;
    }

    outputFile.open(filename, std::ios::binary);
    if (!outputFile.is_open()) {
        return false;
    }

    recording = true;
    totalDataWritten = 0;

    outputFile.seekp(44, std::ios::beg);

    waveInStart(waveIn);
    return true;
}

void AudioRecorder::stopRecording() {
    if (recording) {
        recording = false;

        waveInStop(waveIn);

        if (outputFile.is_open()) {
            WriteWavHeader(outputFile, waveFormat.nSamplesPerSec, waveFormat.wBitsPerSample, waveFormat.nChannels,
                           totalDataWritten);
            outputFile.close();
        }
    }
}
