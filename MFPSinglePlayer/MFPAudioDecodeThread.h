#pragma once
#include "QObject"
#include "MFPVideo.h"
#include "MFPAudioQueue.h"
class MFPAudioDecodeThread:public QObject
{
	Q_OBJECT
private:
	bool isStop;
	MFPVideo* mFPVideo;
	MFPAudioQueue* audioQueue;
public:
	MFPAudioDecodeThread();
	MFPAudioDecodeThread(MFPAudioQueue* audioQueue,MFPVideo* mFPVideo);
	~MFPAudioDecodeThread();
	void setFlag(bool flag);
	bool getFlag();
public slots:
	void decode();
};

