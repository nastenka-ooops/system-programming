//
// Created by madam on 06.12.2024.
//
#include "AudioBuffer.h"

#include <cstdio>
#include <cassert>
#include <Windows.h>

constexpr uint32_t RIFF = 0x46464952;
constexpr uint32_t WAVE = 0x45564157;
constexpr uint32_t FMT = 0x20746d66;
constexpr uint32_t DATA = 0x61746164;

namespace wave {
    struct FileHeader {
        uint32_t chunkId; //"RIFF" (0x52,0x49,0x46,0x46)
        uint32_t chunkSize;
        // (fileSize - 8)  - could also be thought of as bytes of data in file following this field (bytesRemaining)
        uint32_t riffType; // "WAVE" (0x57415645)
    };

    struct FMTChunk {
        uint32_t chunkId; // "fmt " - (0x666D7420)
        uint32_t chunkDataSize; // 16 + extra format bytes
        uint16_t compressionCode; // 1 - PCM
        uint16_t numChannels; // 1 - MONO, 2 - STEREO
        uint32_t sampleRate; //
        uint32_t avgBytesPerSec; //
        uint16_t blockAlign; // 1 - 65535
        uint16_t bitsPerSample; // 2 - 65535
    };

    struct DataChunk {
        uint32_t chunkId; //"data" - 0x64617461
        uint32_t chunkDataSize;
    };
} // namespace wave

AudioBuffer::AudioBuffer()
    : sampleRate(0), bitsPerSample(0), channelCount(0), length(0), data(nullptr) {
}

AudioBuffer::~AudioBuffer() {
    clear();
}

uint8_t *AudioBuffer::create(uint32_t nlength, uint32_t nsampleRate, uint16_t nbitsPerSample, uint16_t nchannelCount) {
    clear();

    sampleRate = nsampleRate;
    bitsPerSample = nbitsPerSample;
    channelCount = nchannelCount;

    length = nlength;
    data = new uint8_t[length];

    return data;
}

bool AudioBuffer::load(const wchar_t *filename) {
    clear();

    wave::FileHeader fileHeader;
    wave::FMTChunk fmtChunk;
    wave::DataChunk dataChunk;
    bool status = false;

    FILE *file = _wfopen(filename, L"rb");
    if (file == nullptr) {
        fclose(file);
        return status;
    }

    if (fread(&fileHeader, sizeof(fileHeader), 1, file) != 1) {
        fclose(file);
        return status;
    }

    if ((fileHeader.chunkId != RIFF) || (fileHeader.riffType != WAVE)) {
        fclose(file);
        return status;
    }

    if (fread(&fmtChunk, sizeof(fmtChunk), 1, file) != 1) {
        fclose(file);
        return status;
    }

    if (fmtChunk.chunkId != FMT) {
        fclose(file);
        return status;
    }

    sampleRate = fmtChunk.sampleRate;
    bitsPerSample = fmtChunk.bitsPerSample;
    channelCount = fmtChunk.numChannels;

    assert(fmtChunk.compressionCode == 1);

    if (fread(&dataChunk, sizeof(dataChunk), 1, file) != 1) {
        fclose(file);
        return status;
    }

    while (dataChunk.chunkId != DATA) {
        if (fseek(file, dataChunk.chunkDataSize, SEEK_CUR) != 0) {
            fclose(file);
            return status;
        }
        if (fread(&dataChunk, sizeof(dataChunk), 1, file) != 1) {
            fclose(file);
            return status;
        }
    }

    if (dataChunk.chunkId != DATA) {
        fclose(file);
        return status;
    }

    length = dataChunk.chunkDataSize;
    data = new uint8_t[dataChunk.chunkDataSize];
    if (fread(data, sizeof(uint8_t), dataChunk.chunkDataSize, file) != dataChunk.chunkDataSize) {
        fclose(file);
        return status;
    }

    status = true;

    fclose(file);
    return status;
}

void AudioBuffer::clear() {
    if (data == nullptr) {
        return;
    }
    sampleRate = 0;
    bitsPerSample = 0;
    channelCount = 0;
    length = 0;
    delete [] data;
}

uint32_t AudioBuffer::getSampleRate() const {
    return sampleRate;
}

uint16_t AudioBuffer::getBitsPerSample() const {
    return bitsPerSample;
}

uint16_t AudioBuffer::getChannelCount() const {
    return channelCount;
}

uint32_t AudioBuffer::getLength() const {
    return length;
}

uint8_t *AudioBuffer::getData() const {
    return data;
}
