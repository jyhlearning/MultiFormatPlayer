#pragma once
#include <core/mat.hpp>

#include "QObject"
#include "MFPVideo.h"
#include "MFPFrameQueue.h"
#include "QImage"
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
	void delay(int msec);
	void playNextFrame(AVFrame* frame);
	void continousPlayBack(AVFrame* frame);
	void clearFrameQueue();
public:
	void setFlag(bool flag);
	MFPlayerThread(MFPFrameQueue* frame);
	~MFPlayerThread();

public slots:
	void onPlay(MFPlayerThreadState::statement sig);
signals:
	void sendFrame(QImage image);
	void stateChange(MFPlayerThreadState::statement state);
	void sendProgress(const qint64 sec);
};
