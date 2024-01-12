#pragma once
#include <core/mat.hpp>

#include "QObject"
#include "MFPVideo.h"
#include "MFPFrameQueue.h"
#include "QImage"
#include "MFPSTDClock.h"
#include "MFPlayerDecodeThread.h"

namespace MFPlayerThreadState {
	enum statement {
		PLAYING,
		PAUSE,
		NEXTFRAME,
		CONTINUEPLAY,
		LASTFRAME
	};
}

class MFPlayerThread : public QObject {
	Q_OBJECT

private:
	bool isStop;
	MFPFrameQueue* frameQueue;
	MFPSTDClock* clock;
	SwsContext* avFrameToQImageSwsContext;
	void delay(int msec);
	int playNextFrame(AVFrame* & frame);
	void continousPlayBack();
	void clearFrameQueue();
	SwsContext* initSwsctx(AVFrame* frame);

public:
	void setFlag(bool flag);
	MFPlayerThread(MFPFrameQueue* frame, MFPSTDClock* clock);
	~MFPlayerThread();

public slots:
	void onPlay(MFPlayerThreadState::statement sig);
signals:
	void sendFrame(QImage image);
	void stateChange(MFPlayerThreadState::statement state);
	void sendProgress(const qint64 sec);
};
