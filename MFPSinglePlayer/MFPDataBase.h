#pragma once
#include <QDir>

#include "QSemaphore"
#include "Qqueue"

template <typename T>
class MFPDataBase {
private:
	QQueue<T> ptr;
	bool quit;
	int capacity;

public:
	QSemaphore freeArea;
	QSemaphore usesArea;

	~MFPDataBase() {
	}

	MFPDataBase(int c = 30) {
		capacity = c;
		quit = false;
	}

	bool isFull() const { return ptr.length() == capacity; }
	bool isEmpty() const { return ptr.length() == 0; }
	bool isQuit() const { return quit; }
	void setQuit(bool flag) { quit = flag; }

	void init() {
		quit = false;
		ptr.clear();
		freeArea.acquire(freeArea.available());
		freeArea.release(capacity-1);
		usesArea.acquire(usesArea.available());
	}

	int safePut(T data) {
		freeArea.acquire();
		if (quit||ptr.size() >= capacity)return -1;
		ptr.enqueue(data);
		usesArea.release();
		return 0;
	}

	int safeGet(T& data) {
		usesArea.acquire();
		if (quit||ptr.size() == 0)return -1;
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

	void forceGetOut() {
		usesArea.release(capacity);
	}
	void forcePutOut() {
		freeArea.release(capacity);
		quit = true;
	}

	void freeLock() {
		int temp = freeArea.available();
		freeArea.release(capacity - temp - 1);
		usesArea.acquire(usesArea.available());
	}
};
