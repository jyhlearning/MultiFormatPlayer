#pragma once
#include <core/mat.hpp>

#include "QObject"
#include "MFPVideo.h"
#include "MFPFrameQueue.h"
#include "QImage"

namespace MFPlayerThreadState {
	enum statement {
		PLAYING,
		PAUSE,
		NEXFRAME,
		CONTINUEPLAY,
		LASTFRAME
	};
	
}
class MFPlayerThread : public QObject {
	Q_OBJECT

private:
	bool isStop;
	MFPFrameQueue<AVFrame>* frameQueue;
	qint64 nowPts;
	void delay(int msec);
	void playNextFrame(AVFrame* frame);
	void continousPlayBack(AVFrame* frame);
public:
	void setFlag(bool flag);
	MFPlayerThread(MFPFrameQueue<AVFrame>* frame);

public slots:
	void onPlay(MFPlayerThreadState::statement sig);
signals:
	void sendFrame(QImage image);
	void stateChange(MFPlayerThreadState::statement state);
	void sendProgress(const qint64 sec,const qint64 totalTime);
};
