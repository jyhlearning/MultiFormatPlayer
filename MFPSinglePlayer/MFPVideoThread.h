#pragma once
//#include <core/mat.hpp>

#include "QObject"
#include "MFPVideo.h"
#include "MFPlayBase.h"
#include "MFPVideoQueue.h"
#include "QImage"
#include "MFPSTDClock.h"

class MFPVideoThread : public MFPlayBase {
	Q_OBJECT

private:
	MFPVideoQueue* frameQueue;
	MFPSTDClock* clock;
	SwsContext* avFrameToQImageSwsContext;
	SwsContext* initSwsctx(AVFrame* frame);
	void io(AVFrame* frame) override;

public:
	MFPVideoThread(MFPVideoQueue* frame, MFPSTDClock* clock);
	~MFPVideoThread();
	void init() override;

signals:
	void sendFrame(QImage image);
};
