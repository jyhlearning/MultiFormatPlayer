#pragma once
#include "MFPDataBase.h"
#include "QMutex"
#include "MFPVideo.h"

class MFPAudioQueue:public MFPDataBase<AVFrame*>
{
private:
	int sampleRate, channels;
	AVSampleFormat sampleFmt;
public:
	bool frameIsEnd;
	QMutex decodeLock, audioLock;

	MFPAudioQueue(int c = 30);

	void initQueue();

	void setChannels(int c);

	int getChannels() const;

	int getSampleRate() const;

	void setSampleRate(int s);

	void setSampleFmt(const AVSampleFormat fmt);

	AVSampleFormat getSampleFmt()const;
};

