#pragma once
#include <QDir>

#include "QMutex"
#include "QSemaphore"
#include "Qqueue"

template <typename T>
class MFPFrameQueue
{
private:
	QQueue<T> ptr;
	AVPixelFormat fmt;
	qint64 capacity;
	qint64 totalTime;
	qint64 lastPts;
	qint64 frameRate;
	double speed;
	bool quit;
public:
	QSemaphore freeArea;
	QSemaphore usesArea;
	QMutex decodeLock,playLock;
	bool frameIsEnd,playEnd ,inited;
	~MFPFrameQueue() {
		
	}

	MFPFrameQueue(int c=30) {
		capacity = c;
		init();
		totalTime = 0;
		frameRate = 0;
		speed = 1;
	}
	bool isFull() const {
		return ptr.length() == capacity;
	}
	bool isEmpty() const {
		return ptr.length() == 0;
	}
	bool isQuit() const {
		return quit;
	}
	void setQuit(bool flag) {
		quit = flag;
	}
	void init() {
		lastPts = 0;
		inited = true;
		quit = false;
		ptr.clear();
		frameIsEnd = false;
		playEnd = true;
		
		int temp = freeArea.available();
		freeArea.release(capacity - temp - 1);
		usesArea.acquire(usesArea.available());
	}

	void setFmt(AVPixelFormat fmt)
	{
		this->fmt = fmt;
	}
	AVPixelFormat getFmt() const
	{
		return fmt;
	}
	void setTotalTime(const qint64 msec){
		totalTime = msec;
	}
	void setLastPts(const qint64 pts) {
		lastPts = pts > totalTime ? totalTime : pts;
		lastPts = pts < 0 ? 0 : lastPts;
	}
	qint64 getTotalTime() const {
		return totalTime;
	}
	qint64 getLastPts() const {
		return lastPts;
	}
	int safePut(T data) {
		freeArea.acquire();
		if (quit)return -1;
		ptr.enqueue(data);
		usesArea.release();
		return 0;
	}

	int safeGet(T &data) {
		usesArea.acquire();
		if (quit)return -1;
		data = ptr.front();
		ptr.pop_front();
		freeArea.release();
		return 0;
	}
	T front() {
		return ptr.front();
	}
	int pop() {
		ptr.pop_front();
		return 0;
	}
	void forceOut() {
		int temp = freeArea.available();
		freeArea.release(capacity - temp - 1);
		usesArea.acquire(usesArea.available());
		quit = true;
	}
	void freeLock() {
		int temp = freeArea.available();
		freeArea.release(capacity - temp - 1);
		usesArea.acquire(usesArea.available());
	}
	void setFrameRate(qint64 rate) {
		frameRate = rate;
	}
	qint64 getFrameRate() const{
		return frameRate;
	}
	void setSpeed(double s) {
		speed = s;
	}
	double getSpeed() const {
		return speed;
	}
};

