#pragma once
#include "qobject.h"
#include "MFPVideo.h"
#include "MFPFrameQueue.h"
#include "MFPAudioQueue.h"
class MFPlayerDecodeThread : public QObject {
	Q_OBJECT

public:
	MFPlayerDecodeThread();
	MFPlayerDecodeThread(MFPFrameQueue* frameQueue, MFPAudioQueue* audioQueue,MFPVideo* mFPVideo);
	~MFPlayerDecodeThread();

	void setFlag(bool flag);
	bool getFlag();

public slots:
	void decode();

private:
	bool isStop;
	MFPVideo* mFPVideo;
	MFPFrameQueue* frameQueue;
	MFPAudioQueue* audioQueue;
};
