#include "MFPFrameQueue.h"

MFPFrameQueue::~MFPFrameQueue() {
}

MFPFrameQueue::MFPFrameQueue(int c) : MFPDataBase<AVFrame*>(c) {
	initQueue();
	totalTime = 0;
	frameRate = 0;
	speed = 1;
	lastPts = 0;
}

void MFPFrameQueue::initQueue() {
	frameIsEnd = false;
	init();
}

void MFPFrameQueue::setTotalTime(const qint64 msec) { totalTime = msec; }

void MFPFrameQueue::setLastPts(const qint64 pts) {
	lastPts = pts > totalTime ? totalTime : pts;
	lastPts = pts < 0 ? 0 : lastPts;
}

qint64 MFPFrameQueue::getTotalTime() const { return totalTime; }
qint64 MFPFrameQueue::getLastPts() const { return lastPts; }
void MFPFrameQueue::setFrameRate(qint64 rate) { frameRate = rate; }
qint64 MFPFrameQueue::getFrameRate() const { return frameRate; }
void MFPFrameQueue::setSpeed(double s) { speed = s; }
double MFPFrameQueue::getSpeed() const { return speed; }
