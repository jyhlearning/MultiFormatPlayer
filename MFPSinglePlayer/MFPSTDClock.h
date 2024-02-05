#pragma once
#include "QMutex"
#include "QTime"
#include "QElapsedTimer"
class MFPSTDClock
{
private:
	QElapsedTimer timer;
	QMutex lock;
	qint64 lPts,totalTime,firstPts,startPts;
	bool isSet;
public:
	MFPSTDClock();
	void init();
	void StartTimer();
	void setStartTime(const qint64 t);
	void setTotalTime(const qint64 t);
	void setLastPts(const qint64 t);
	void setFirstPts(const qint64 t);
	void setStartPts(const qint64 t);
	qint64 getLastPts() const;
	qint64 getFirstPts() const;
	qint64 getDelayTime(const qint64 pts) const;
	qint64 getTotalTime() const;
	qint64 getDeltaTimer() const;
	qint64 getStartPts() const;
	static void delay(const qint64 msec, const qint64 frameRate);
};

