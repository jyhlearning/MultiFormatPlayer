#include "MFPAudioQueue.h"

MFPAudioQueue::MFPAudioQueue(int c) : MFPDataBase<AVFrame*>(c) {
	channels = 2;
	sampleRate = 44100;
	initQueue();
}

MFPAudioQueue::~MFPAudioQueue() { }

void MFPAudioQueue::initQueue() {
	init();
}

void MFPAudioQueue::setChannels(int c) { channels = c; }

int MFPAudioQueue::getChannels() const { return channels; }

int MFPAudioQueue::getSampleRate() const { return sampleRate; }

void MFPAudioQueue::setSampleRate(int s) { sampleRate = s; }

void MFPAudioQueue::setSampleFmt(const AVSampleFormat fmt) { sampleFmt = fmt; }

AVSampleFormat MFPAudioQueue::getSampleFmt() const { return sampleFmt; }
