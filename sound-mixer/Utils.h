#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <fstream>

void WriteWavHeader(std::ofstream &file, uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels, uint32_t dataSize);

#endif //UTILS_H
