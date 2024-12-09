#include "AudioMixer.h"
#include <cstdio>
#include "Utils.h"

static bool WaveOutSetVolume(HWAVEOUT waveOut, float volume, float pan) {
    const uint16_t leftChannel = (uint16_t) ((float) 0xFFFF * volume * pan);
    const uint16_t rightChannel = (uint16_t) ((float) 0xFFFF * volume * (1.0f - pan));
    return (waveOutSetVolume(waveOut, leftChannel << 16 | rightChannel) == MMSYSERR_NOERROR);
}

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    AudioMixer *mixer = (AudioMixer *) dwInstance;

    switch (uMsg) {
        case WOM_DONE:
            ++mixer->freeBlocks;
            pthread_cond_signal(&mixer->freeBlocksCond);
            break;
    }
}

void *AudioThreadProc(void *params) {
    AudioMixer *mixer = (AudioMixer *) params;

    while (mixer->ready) {
        if (mixer->freeBlocks == 0) {
            pthread_mutex_lock(&mixer->freeBlocksMutex);
            pthread_cond_wait(&mixer->freeBlocksCond, &mixer->freeBlocksMutex);
        }
        --mixer->freeBlocks;

        memset(mixer->accumulator, 0, sizeof(int32_t) * mixer->blockSize);

        for (uint32_t index = 0; index < mixer->sources.size(); ++index) {
            AudioSource *source = mixer->sources[index];
            const AudioBuffer *buffer = source->getBuffer();
            const uint8_t *sampleBuffer = buffer->getData();

            const int32_t active = (source->getStatus() == AudioSource::PLAY);
            int32_t position;

            if (source->getPosition() >= buffer->getLength()) {
                source->stop();
                if (source->getLoop()) {
                    source->play();
                }
                position = 0;
            } else {
                position = source->getPosition();
                float pan = source->getPan();
                float leftFactor = 1.0f - pan;
                float rightFactor = pan;

                for (uint32_t sampleIndex = 0; sampleIndex < mixer->blockSize / 2; sampleIndex += 2) {
                    if (source->getStatus() == AudioSource::PLAY) {
                        int16_t *sampleData = (int16_t *) (sampleBuffer + position);
                        mixer->accumulator[sampleIndex] += (int32_t) (
                            sampleData[sampleIndex] * source->getVolume() * leftFactor * 10);
                        mixer->accumulator[sampleIndex + 1] += (int32_t) (
                            sampleData[sampleIndex] * source->getVolume() * rightFactor * 10);
                    }
                }
            }

            source->setPosition(position + (float) (mixer->blockSize * active) * source->getSpeed());
        }

        for (uint32_t index = 0; index < mixer->blockSize / 2; ++index) {
            ((int16_t *) (mixer->waveHeaderBuffer + mixer->blockIndex * mixer->blockSize))[index] =
                    mixer->accumulator[index] / AUDIO_MIXER_MAX_SOURCE_COUNT;
        }

        // Запись данных в WAV-файл
        if (mixer->outputFile.is_open()) {
            mixer->outputFile.write(
                (char *) (mixer->waveHeaderBuffer + mixer->blockIndex * mixer->blockSize),
                mixer->blockSize
            );
            mixer->totalDataWritten += mixer->blockSize;
        }

        waveOutWrite(mixer->waveOut, &mixer->waveHeaders[mixer->blockIndex], sizeof(WAVEHDR));
        mixer->blockIndex = (mixer->blockIndex + 1) % mixer->blockCount;

        pthread_mutex_unlock(&mixer->freeBlocksMutex);
    }

    return params;
}

bool AudioMixer::create(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels, uint8_t nblockCount,
                        uint32_t nchunkSize) {
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nSamplesPerSec = samplesPerSec;
    waveFormat.wBitsPerSample = bitsPerSample;
    waveFormat.nChannels = channelCount;
    waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    waveOutSetVolume(waveOut, 0xFFFFFFFF);
    waveOutOpen(&waveOut, WAVE_MAPPER, &waveFormat, (DWORD_PTR) waveOutProc, (DWORD_PTR) this, CALLBACK_FUNCTION);

    blockCount = nblockCount;
    blockSize = nchunkSize;

    waveHeaderBuffer = new uint8_t[blockCount * blockSize];
    accumulator = new int32_t[blockSize];
    waveHeaders = new WAVEHDR[blockCount];

    for (uint32_t index = 0; index < blockCount; ++index) {
        waveHeaders[index].lpData = (char *) (waveHeaderBuffer + index * blockSize);
        waveHeaders[index].dwBufferLength = blockSize;
        waveHeaders[index].dwFlags = 0;
        waveHeaders[index].dwLoops = 0;

        memset(waveHeaders[index].lpData, 0, blockSize);

        waveOutPrepareHeader(waveOut, &waveHeaders[index], sizeof(WAVEHDR));
        waveOutWrite(waveOut, &waveHeaders[index], sizeof(WAVEHDR));
    }

    freeBlocks = blockCount;

    ready = true;
    blockIndex = 0;
    freeBlocksMutex = PTHREAD_MUTEX_INITIALIZER;
    freeBlocksCond = PTHREAD_COND_INITIALIZER;
    pthread_create(&audioThread, nullptr, AudioThreadProc, this);

    return true;
}

void AudioMixer::destroy() {
    ready = false;
    pthread_join(audioThread, nullptr);

    for (uint32_t index = 0; index < blockCount; ++index) {
        waveOutUnprepareHeader(waveOut, &waveHeaders[index], sizeof(WAVEHDR));
    }
    waveOutClose(waveOut);

    if (waveHeaders != nullptr) {
        delete [] waveHeaders;
    }
    if (accumulator != nullptr) {
        delete [] accumulator;
    }
    if (waveHeaderBuffer != nullptr) {
        delete [] waveHeaderBuffer;
    }
}

AudioMixer::AudioMixer(uint32_t samplesPerSec, uint16_t bitsPerSample, uint16_t channelCount, uint8_t nblockCount,
                       uint32_t nchunkSize)
    : samplesPerSec(samplesPerSec), bitsPerSample(bitsPerSample), channelCount(channelCount) {
    create(samplesPerSec, bitsPerSample, channelCount, nblockCount, nchunkSize);
}

AudioMixer::~AudioMixer() {
    destroy();
}

AudioSource *AudioMixer::create(AudioBuffer *audioBuffer, const wchar_t *name) {
    AudioSource *source = new AudioSource(audioBuffer, name);
    sources.push_back(source);
    return source;
}

AudioSource *AudioMixer::create(const wchar_t *filename) {
    AudioBuffer *audioBuffer = new AudioBuffer();
    if (audioBuffer->load(filename) == false) {
        delete audioBuffer;
        return nullptr;
    }
    return create(audioBuffer, filename);
}

AudioSource *AudioMixer::play(AudioBuffer *audioBuffer) {
    AudioSource *source = create(audioBuffer, L"new");
    source->play();
    return source;
}

AudioSource *AudioMixer::play(const wchar_t *filename) {
    AudioSource *source = create(filename);
    if (source != nullptr) {
        source->play();
    }
    return source;
}

std::vector<AudioSource *> AudioMixer::getSources() {
    return sources;
}

bool AudioMixer::startRecording(const std::string &filename) {
    if (outputFile.is_open()) {
        return false; // Запись уже началась
    }

    outputFile.open(filename, std::ios::binary);
    if (!outputFile.is_open()) {
        return false;
    }

    totalDataWritten = 0;

    // Пропустить 44 байта для заголовка WAV
    outputFile.seekp(44, std::ios::beg);
    return true;
}

void AudioMixer::stopRecording() {
    if (outputFile.is_open()) {
        // Обновить заголовок файла с учетом записанных данных
        WriteWavHeader(outputFile, samplesPerSec, bitsPerSample, channelCount, totalDataWritten);
        outputFile.close();
    }
}
