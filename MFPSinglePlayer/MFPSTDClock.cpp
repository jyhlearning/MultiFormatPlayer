#include "MFPSTDClock.h"

#include <thread>

MFPSTDClock::MFPSTDClock() {
	isSet = false;
	lPts = 0;
	firstPts = 0;
	startPts = 0;
}

void MFPSTDClock::init() {
	isSet = false;
}

void MFPSTDClock::StartTimer() { timer.restart(); }

void MFPSTDClock::setStartTime(const qint64 t) {
	if (isSet)return;
	if (!lock.tryLock())return;
	StartTimer();
	setFirstPts(t);
	isSet = true;
	lock.unlock();
}

void MFPSTDClock::setTotalTime(const qint64 t) { totalTime = t; }

void MFPSTDClock::setLastPts(const qint64 t) {
	lPts = t < startPts ? startPts : t;
	lPts = lPts < totalTime ? lPts : totalTime;
}

void MFPSTDClock::setFirstPts(const qint64 t) { firstPts = t; }

void MFPSTDClock::setStartPts(const qint64 t) { startPts = t; }

qint64 MFPSTDClock::getLastPts() const { return lPts; }

qint64 MFPSTDClock::getFirstPts() const { return firstPts; }

qint64 MFPSTDClock::getDelayTime(const qint64 pts) const { return pts - getFirstPts() - getDeltaTimer(); }

qint64 MFPSTDClock::getTotalTime() const { return totalTime; }

qint64 MFPSTDClock::getDeltaTimer() const {
	const qint64 a = timer.elapsed();
	return a < 0 ? 0 : a;
}

qint64 MFPSTDClock::getStartPts() const { return startPts; }

void MFPSTDClock::delay(const qint64 msec, const qint64 frameRate) {
	if (msec <= 0)return;
	const qint64 a = msec > 500 ? 1000 / frameRate : msec;
	std::this_thread::sleep_for(std::chrono::milliseconds(a));
}
