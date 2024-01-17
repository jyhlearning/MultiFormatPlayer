#pragma once

#include "QMutex"
#include "MFPDataBase.h"
#include <libswscale/swscale.h>

class MFPFrameQueue : public MFPDataBase<AVFrame*> {
private:

public:
	QMutex decodeLock, playLock;
	bool frameIsEnd;

	~MFPFrameQueue();

	MFPFrameQueue(int c = 30);

	void initQueue();
};
