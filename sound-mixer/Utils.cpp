//
// Created by madam on 09.12.2024.
//

#include "Utils.h"

void WriteWavHeader(std::ofstream &file, uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels, uint32_t dataSize) {
    file.seekp(0, std::ios::beg); // Установить указатель в начало файла
    file.write("RIFF", 4); // RIFF Header
    uint32_t chunkSize = 36 + dataSize; // Размер всего файла - 8 байт
    file.write(reinterpret_cast<const char *>(&chunkSize), 4);
    file.write("WAVE", 4); // WAVE Header

    file.write("fmt ", 4); // Format chunk
    uint32_t subchunk1Size = 16; // PCM header size
    file.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
    uint16_t audioFormat = 1; // PCM format
    file.write(reinterpret_cast<const char *>(&audioFormat), 2);
    file.write(reinterpret_cast<const char *>(&channels), 2);
    file.write(reinterpret_cast<const char *>(&sampleRate), 4);
    uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
    file.write(reinterpret_cast<const char *>(&byteRate), 4);
    uint16_t blockAlign = channels * (bitsPerSample / 8);
    file.write(reinterpret_cast<const char *>(&blockAlign), 2);
    file.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

    file.write("data", 4); // Data chunk
    file.write(reinterpret_cast<const char *>(&dataSize), 4);
}
