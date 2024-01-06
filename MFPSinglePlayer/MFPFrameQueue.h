#pragma once

#include "QMutex"
#include "MFPDataBase.h"
#include <libavcodec/avcodec.h>

class MFPFrameQueue : public MFPDataBase<AVFrame> {
private:
	AVPixelFormat fmt;
	qint64 totalTime;
	qint64 lastPts;
	qint64 frameRate;
	double speed;

public:
	QMutex decodeLock, playLock;
	bool frameIsEnd;

	~MFPFrameQueue();

	MFPFrameQueue(int c = 30);

	void initQueue();

	void setFmt(AVPixelFormat fmt);

	AVPixelFormat getFmt() const;

	void setTotalTime(const qint64 msec);

	qint64 getTotalTime() const;

	void setLastPts(const qint64 pts);

	qint64 getLastPts() const;

	void setFrameRate(qint64 rate);

	qint64 getFrameRate() const;

	void setSpeed(double s);

	double getSpeed() const;
};
