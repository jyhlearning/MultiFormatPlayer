#pragma once

#include "QSemaphore"
#include "Qqueue"
#include "QMutex"
extern "C" {
#include <libswresample/swresample.h>
}
template <typename T>
class MFPDataBase {
private:
	QQueue<T> ptr;
	bool quit, over;
	int capacity;
	qint64 frameRate;
	double speed;
	QSemaphore freeArea;
	QSemaphore usesArea;
	bool frameIsEnd;
public:
	QMutex decodeLock, frameLock;
	~MFPDataBase() {
	}

	MFPDataBase(int c = 30) {
		capacity = c;
		quit = false;
		over = false;
		frameRate = 0;
		speed = 1;
	}

	bool isFull() const { return ptr.length() == capacity; }
	bool isEmpty() const { return ptr.length() == 0; }
	bool isQuit() const { return quit; }
	void setQuit(bool flag) { quit = flag; }

	void init() {
		clearQueue();
		ptr.clear();
		quit = false;
		frameIsEnd = false;
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

	void setCapacity(const qint64 c) { capacity = c; };

	void setFrameRate(qint64 rate) { frameRate = rate; }
	qint64 getFrameRate() const { return frameRate; }
	void setSpeed(double s) { speed = s; }
	double getSpeed() const { return speed; }
	void setPlayEnd(bool flag) { over = flag; }
	bool getPlayEnd() const { return over; }
	void clearQueue() {
		//由播放器负责清理
		//audioQueue->playLock.lock();
		decodeLock.lock();
		if (!isEmpty()) {
			while (!isEmpty()) {
				AVFrame* frame = front();
				av_frame_free(&frame);
				pop();
			}
		}
		decodeLock.unlock();
		//audioQueue->playLock.unlock();
	}
	void setIsEnd(bool b) { frameIsEnd = b; }
	bool getIsEnd()const { return frameIsEnd; }
};
