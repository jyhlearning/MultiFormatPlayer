#include "MFPFrameQueue.h"

MFPFrameQueue::~MFPFrameQueue() {
}

MFPFrameQueue::MFPFrameQueue(int c) : MFPDataBase<AVFrame*>(c) {
	initQueue();
}

void MFPFrameQueue::initQueue() {
	frameIsEnd = false;
	init();
}
