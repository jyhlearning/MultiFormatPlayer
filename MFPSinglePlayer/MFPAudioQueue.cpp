#include "MFPAudioQueue.h"

MFPAudioQueue::MFPAudioQueue(int c) : MFPDataBase<AVFrame*>(c) {
	channels = 2;
	sampleRate = 44100;
	lastPts = 0;
	frameRate = 40;
	speed = 1;
	initQueue();
}

void MFPAudioQueue::initQueue() {
	frameIsEnd = false;
	init();
}

void MFPAudioQueue::setLastPts(const qint64 pts) { lastPts = pts; }

qint64 MFPAudioQueue::getLastPts() const { return lastPts; }

void MFPAudioQueue::setFrameRate(qint64 rate) { frameRate = rate; }

qint64 MFPAudioQueue::getFrameRate() const { return frameRate; }

void MFPAudioQueue::setSpeed(double s) { speed = s; }

double MFPAudioQueue::getSpeed() const { return speed; }

void MFPAudioQueue::setSwrctx(SwrContext* swr_ctx) { this->swr_ctx = swr_ctx; }

void MFPAudioQueue::setChannels(int c) { channels = c; }

int MFPAudioQueue::getChannels() const { return channels; }

int MFPAudioQueue::getSampleRate() const { return sampleRate; }

void MFPAudioQueue::setSampleRate(int s) { sampleRate = s; }

SwrContext* MFPAudioQueue::getSwrctx() { return swr_ctx; }
