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
	qint64 capacity;
	AVPixelFormat fmt;
public:
	QSemaphore freeArea;
	QSemaphore usesArea;
	bool frameIsEnd,playEnd;
	~MFPFrameQueue() {
		
	}

	MFPFrameQueue(int c=30) {
		capacity = c;
		init();
	}
	bool isFull() const {
		return ptr.length() == capacity;
	}
	bool isEmpty() const {
		return ptr.length() == 0;
	}
	void init() {
		ptr.clear();
		frameIsEnd = false;
		playEnd = true;
		int temp = freeArea.available();
		freeArea.release(capacity - temp-1);
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


	int safePut(T data) {
		freeArea.acquire();
		ptr.enqueue(data);
		usesArea.release();
		return 0;
	}

	int safeGet(T &data) {
		usesArea.acquire();
		data = ptr.front();
		ptr.pop_front();
		freeArea.release();
		return 0;
	}
	T front() {
		return ptr.front();;
	}
	int pop() {
		ptr.pop_front();
		return 0;
	}

};

