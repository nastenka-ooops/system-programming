#include "AudioMixer.h"

#include <stdio.h>

static bool WaveOutSetVolume(HWAVEOUT waveOut, float volume, float pan) {
	const uint16_t leftChannel  = (uint16_t)((float)0xFFFF * volume * pan);
	const uint16_t rightChannel = (uint16_t)((float)0xFFFF * volume * (1.0f - pan));
	return (waveOutSetVolume(waveOut, leftChannel << 16 | rightChannel) == MMSYSERR_NOERROR);
}

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	AudioMixer* mixer = (AudioMixer*)dwInstance;

	switch (uMsg) {
	case WOM_DONE:
		++mixer->freeBlocks;
		pthread_cond_signal(&mixer->freeBlocksCond);
		break;
	}
}

void* AudioThreadProc(void* params) {
	AudioMixer* mixer = (AudioMixer*)params;

	while(mixer->ready) {
		if (mixer->freeBlocks == 0) {
			pthread_mutex_lock(&mixer->freeBlocksMutex);
			pthread_cond_wait(&mixer->freeBlocksCond, &mixer->freeBlocksMutex);
		}
		--mixer->freeBlocks;

		memset(mixer->accumulator, 0, sizeof(int32_t) * mixer->blockSize);

		for (uint32_t index = 0; index < mixer->sources.size(); ++index) {
			AudioSource* source = mixer->sources[index];
			const AudioBuffer* buffer = source->getBuffer();
			const uint8_t* sampleBuffer = buffer->getData();

			const int32_t active = (source->getStatus() == AudioSource::PLAY);
			const int32_t position = source->getPosition();

			if (source->getPosition() >= buffer->getLength()) {
				source->stop();
				if (source->getLoop()) {
					source->play();
				}
			} else {
				for (uint32_t sampleIndex = 0; sampleIndex < mixer->blockSize / 2; ++sampleIndex) {
					mixer->accumulator[sampleIndex] += (active * ((int16_t*)(sampleBuffer + position))[sampleIndex] * source->getVolume());
				}
			}

			source->setPosition(position + (float)(mixer->blockSize * active) * source->getSpeed());
		}

		//if (mixer->waveHeaders[mixer->blockIndex].dwFlags & WHDR_PREPARED) {
		//	waveOutUnprepareHeader(mixer->waveOut, &mixer->waveHeaders[mixer->blockIndex], sizeof(WAVEHDR));
		//}

		for (uint32_t index = 0; index < mixer->blockSize / 2; ++index) {
			((int16_t*)(mixer->waveHeaderBuffer + mixer->blockIndex * mixer->blockSize))[index] = mixer->accumulator[index] / AUDIO_MIXER_MAX_SOURCE_COUNT;
		}

		// waveOutPrepareHeader(mixer->waveOut, &mixer->waveHeaders[mixer->blockIndex], sizeof(WAVEHDR));
		waveOutWrite(mixer->waveOut, &mixer->waveHeaders[mixer->blockIndex], sizeof(WAVEHDR));

		mixer->blockIndex = (mixer->blockIndex + 1) % mixer->blockCount;

		pthread_mutex_unlock(&mixer->freeBlocksMutex);
	}

	return params;
}

bool AudioMixer::create(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels, uint8_t nblockCount, uint32_t nchunkSize) {
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = samplesPerSec;
	waveFormat.wBitsPerSample = bitsPerSample;
	waveFormat.nChannels = channelCount;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	waveOutOpen(&waveOut, WAVE_MAPPER, &waveFormat, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	waveOutSetVolume(waveOut, 0xFFFFFFFF);

	blockCount = nblockCount;
	blockSize = nchunkSize;

	waveHeaderBuffer = new uint8_t[blockCount * blockSize];
	accumulator = new int32_t[blockSize];
	waveHeaders = new WAVEHDR[blockCount];

	for (uint32_t index = 0; index < blockCount; ++index) {
		waveHeaders[index].lpData = (char*)(waveHeaderBuffer + index * blockSize);
		waveHeaders[index].dwBufferLength = blockSize;
		waveHeaders[index].dwFlags = 0;
		waveHeaders[index].dwLoops = 0;

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

AudioMixer::AudioMixer(uint32_t samplesPerSec, uint16_t bitsPerSample, uint16_t channelCount, uint8_t nblockCount, uint32_t nchunkSize)
	: samplesPerSec(samplesPerSec), bitsPerSample(bitsPerSample), channelCount(channelCount) {
	create(samplesPerSec, bitsPerSample, channelCount, nblockCount, nchunkSize);
}

AudioMixer::~AudioMixer() {
	destroy();
}

AudioSource* AudioMixer::create(AudioBuffer* audioBuffer) {
	AudioSource* source = new AudioSource(audioBuffer);
	sources.push_back(source);
	return source;
}

AudioSource* AudioMixer::create(const char* filename) {
	AudioBuffer* audioBuffer = new AudioBuffer();
	if (audioBuffer->load(filename) == false) {
		delete audioBuffer;
		return nullptr;
	}
	return create(audioBuffer);
}

AudioSource* AudioMixer::play(AudioBuffer* audioBuffer) {
	AudioSource* source = create(audioBuffer);
	source->play();
	return source;
}

AudioSource* AudioMixer::play(const char* filename) {
	AudioSource* source = create(filename);
	if (source != nullptr) {
		source->play();
	}
	return source;
}
