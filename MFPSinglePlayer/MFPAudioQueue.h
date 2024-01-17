#pragma once
#include "MFPDataBase.h"
#include "QMutex"
#include "MFPVideo.h"

class MFPAudioQueue:public MFPDataBase<AVFrame*>
{
private:
	int sampleRate, channels;
	qint64 lastPts;
	qint64 frameRate;
	qint64 totalTime;
	double speed;
	AVSampleFormat sampleFmt;
public:
	bool frameIsEnd;
	QMutex decodeLock, audioLock;

	MFPAudioQueue(int c = 30);

	void initQueue();

	void setLastPts(const qint64 pts);

	qint64 getLastPts() const;

	void setFrameRate(qint64 rate);

	qint64 getFrameRate() const;

	void setSpeed(double s);

	double getSpeed() const;

	void setChannels(int c);

	int getChannels() const;

	int getSampleRate() const;

	void setSampleRate(int s);

	void setSampleFmt(const AVSampleFormat fmt);

	AVSampleFormat getSampleFmt()const;
};

