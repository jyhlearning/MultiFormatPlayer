#pragma once
#include <core/mat.hpp>

#include "QObject"
#include "MFPVideo.h"
#include "MFPFrameQueue.h"
#include "QImage"

enum statement {
	PLAYING,
	PAUSE
};
class MFPlayerThread : public QObject {
	Q_OBJECT

private:
	bool isStop;
	MFPFrameQueue<AVFrame>* frameQueue;
	void delay(int msec);
public:
	void setFlag(bool flag);
	MFPlayerThread(MFPFrameQueue<AVFrame>* frame);

public slots:
	void onPlay();
signals:
	void sendFrame(QImage image);
	void stateChange(statement state);
};
