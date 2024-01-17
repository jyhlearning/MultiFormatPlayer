#pragma once
#include <QDir>

#include "QSemaphore"
#include "Qqueue"

template <typename T>
class MFPDataBase {
private:
	QQueue<T> ptr;
	bool quit, over;
	int capacity;
	qint64 startTime;
	qint64 totalTime;
	qint64 lastPts;
	qint64 frameRate;
	double speed;

public:
	QSemaphore freeArea;
	QSemaphore usesArea;

	~MFPDataBase() {
	}

	MFPDataBase(int c = 30) {
		capacity = c;
		quit = false;
		over = false;
		totalTime = 0;
		frameRate = 0;
		speed = 1;
		startTime = 0;
		lastPts = 0;
	}

	bool isFull() const { return ptr.length() == capacity; }
	bool isEmpty() const { return ptr.length() == 0; }
	bool isQuit() const { return quit; }
	void setQuit(bool flag) { quit = flag; }

	void init() {
		quit = false;
		ptr.clear();
		freeArea.acquire(freeArea.available());
		freeArea.release(capacity - 1);
		usesArea.acquire(usesArea.available());
	}

	int safePut(T data) {
		freeArea.acquire();
		if (quit || ptr.size() >= capacity)return -1;
		ptr.enqueue(data);
		usesArea.release();
		return 0;
	}

	int safeGet(T& data) {
		usesArea.acquire();
		if (quit || ptr.size() == 0)return -1;
		data = ptr.front();
		ptr.pop_front();
		freeArea.release();
		return 0;
	}

	T front() { return ptr.front(); }

	int pop() {
		ptr.pop_front();
		return 0;
	}

	void forceGetOut() { usesArea.release(capacity); }

	void forcePutOut() {
		freeArea.release(capacity);
		quit = true;
	}

	void freeLock() {
		int temp = freeArea.available();
		freeArea.release(capacity - temp - 1);
		usesArea.acquire(usesArea.available());
	}


	void setLastPts(const qint64 pts) {
		lastPts = pts > totalTime ? totalTime : pts;
		lastPts = pts < startTime ? startTime : lastPts;
	}

	void setTotalTime(const qint64 msec) { totalTime = msec; }
	qint64 getTotalTime() const { return totalTime; }
	qint64 getLastPts() const { return lastPts; }
	void setFrameRate(qint64 rate) { frameRate = rate; }
	qint64 getFrameRate() const { return frameRate; }
	void setSpeed(double s) { speed = s; }
	double getSpeed() const { return speed; }
	void setStartTime(qint64 time) { startTime = time; }
	qint64 getStartTime() const { return startTime; }
	void setPlayEnd(bool flag) { over = flag; }
	bool getPlayEnd() const { return over; }
};
