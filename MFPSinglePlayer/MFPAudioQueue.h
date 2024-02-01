#pragma once
#include "MFPDataBase.h"
class MFPAudioQueue:public MFPDataBase<AVFrame*>
{
private:
	int sampleRate, channels;
	AVSampleFormat sampleFmt;
public:

	MFPAudioQueue(int c = 30);

	~MFPAudioQueue();

	void initQueue();

	void setChannels(int c);

	int getChannels() const;

	int getSampleRate() const;

	void setSampleRate(int s);

	void setSampleFmt(const AVSampleFormat fmt);

	AVSampleFormat getSampleFmt()const;
};

