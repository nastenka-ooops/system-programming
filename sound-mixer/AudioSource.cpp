//
// Created by madam on 06.12.2024.
//

#include "AudioSource.h"

#include <algorithm>

AudioSource::AudioSource(AudioBuffer *newBuffer)
	: buffer(nullptr), position(0), volume(1.0), pan(0.5f), speed(1.0f), status(STOP), loop(false) {
	setBuffer(newBuffer);
}

AudioSource::~AudioSource() {
	setBuffer(nullptr);
}

void AudioSource::setBuffer(AudioBuffer *value) {
	buffer = value;
	position = 0;
	status = STOP;
}

AudioBuffer *AudioSource::getBuffer() const {
	return buffer;
}

bool AudioSource::play() {
	switch (status) {
		case STOP:
			status = PLAY;
			position = 0;
			return true;
		case PAUSE:
			status = PLAY;
			return true;
	}
	return false;
}

bool AudioSource::pause() {
	switch (status) {
		case PLAY:
			status = PAUSE;
			return true;
	}
	return false;
}

bool AudioSource::stop() {
	switch (status) {
		case PLAY:
		case PAUSE:
			status = STOP;
			position = 0;
			return true;
	}
	return false;
}

bool AudioSource::finished() {
	return (status == STOP);
}

uint8_t AudioSource::getStatus() const {
	return status;
}

double AudioSource::getElapsedSeconds() const {
	return (buffer == nullptr) ? 0.0 : position / buffer->getSampleRate() / buffer->getChannelCount() / 2;
}

double AudioSource::getTotalSeconds() const {
	return (buffer == nullptr) ? 0.0 : getSampleCount() / buffer->getSampleRate();
}

uint32_t AudioSource::getSampleCount() const {
	if (buffer == nullptr) {
		return 0;
	}

	if (buffer->getLength() == 0) {
		return 0;
	}

	if (buffer->getBitsPerSample() == 0) {
		return 0;
	}

	if (buffer->getChannelCount() == 0) {
		return 0;
	}

	return (buffer->getLength() / (buffer->getBitsPerSample() / 8) / buffer->getChannelCount());
}

void AudioSource::setPosition(uint32_t value) {
	if (buffer == nullptr) {
		return;
	}

	position = std::max(0u, std::min(buffer->getLength(), value));
	position = (position >> 1) << 1;

	if (status == STOP) {
		status = PAUSE;
	}
	if (position >= buffer->getLength()) {
		status = STOP;
	}
}

uint32_t AudioSource::getPosition() const {
	return position;
}

void AudioSource::setProgress(float value) {
	if (buffer == nullptr) {
		return;
	}

	position = (float) buffer->getLength() * std::min(1.0f, std::max(0.0f, value));
	position = (position >> 1) << 1; // align to the channel and byte count
	position = std::min(position, buffer->getLength());

	if (status == STOP) {
		status = PAUSE;
	}
}

float AudioSource::getProgress() const {
	return (buffer == nullptr) ? 0.0f : (float) position / (float) buffer->getLength();
}

void AudioSource::setLoop(bool value) {
	loop = value;
}

bool AudioSource::getLoop() const {
	return loop;
}

void AudioSource::setVolume(float value) {
	volume = std::max(0.0f, std::min(1.0f, value));
}

float AudioSource::getVolume() const {
	return volume;
}

void AudioSource::setPan(float value) {
	pan = std::max(0.0f, std::min(1.0f, value));;
}

float AudioSource::getPan() const {
	return pan;
}

void AudioSource::setSpeed(float value) {
	speed = value;
}

float AudioSource::getSpeed() const {
	return speed;
}
