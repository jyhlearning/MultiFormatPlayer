#pragma once

#include "MFPDataBase.h"

class MFPVideoQueue : public MFPDataBase<AVFrame*> {
public:
	~MFPVideoQueue() = default;

	MFPVideoQueue(int c = 30);

	void initQueue();
};
