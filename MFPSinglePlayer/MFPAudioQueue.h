#pragma once
#include "MFPDataBase.h"
#include "QMutex"
#include "MFPVideo.h"

class MFPAudioQueue:public MFPDataBase<AVFrame>
{
private:
	int sampleRate, channels;
	qint64 lastPts;
	qint64 frameRate;
	SwrContext* swr_ctx;
	double speed;
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

	void setSwrctx(SwrContext* swr_ctx);
	SwrContext* getSwrctx();
};

