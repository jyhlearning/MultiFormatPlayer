#include "MFPVideoQueue.h"

MFPVideoQueue::MFPVideoQueue(int c) : MFPDataBase<AVFrame*>(c) { initQueue(); }

void MFPVideoQueue::initQueue() {
	init();
}