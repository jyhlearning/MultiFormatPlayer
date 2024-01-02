#pragma once
#include "qobject.h"
#include "opencv2/highgui/highgui.hpp"
#include "MFPVideo.h"
#include "MFPFrameQueue.h"

class MFPlayerDecodeThread:public QObject 
{
	Q_OBJECT
public:
	MFPlayerDecodeThread();
	MFPlayerDecodeThread(MFPFrameQueue<AVFrame>* frameQueue);
	~MFPlayerDecodeThread();

	void setFlag(bool flag);
	bool getFlag();
public slots:
	void decode();
	void onControlProgress(int msec);
private:
	bool isStop;
	MFPVideo* mFPVideo;
	MFPFrameQueue<AVFrame>* frameQueue;
	void clearFrameQueue();
};

