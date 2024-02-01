#pragma once

#include "QMutex"
#include "MFPDataBase.h"
class MFPFrameQueue : public MFPDataBase<AVFrame*> {
private:
	void clearFrameQueue();

public:
	~MFPFrameQueue();

	MFPFrameQueue(int c = 30);

	void initQueue();
};
