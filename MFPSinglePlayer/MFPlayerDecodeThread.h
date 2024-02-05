#pragma once
#include "qobject.h"
#include "MFPVideo.h"
#include "MFPVideoQueue.h"
#include "MFPAudioQueue.h"
class MFPlayerDecodeThread : public QObject {
	Q_OBJECT

public:
	MFPlayerDecodeThread();
	MFPlayerDecodeThread(MFPVideoQueue* frameQueue, MFPAudioQueue* audioQueue,MFPVideo* mFPVideo);
	~MFPlayerDecodeThread();

	void setFlag(bool flag);
	bool getFlag();

public slots:
	void decode(const int option, const qint64 lPts);

private:
	bool isStop;
	MFPVideo* mFPVideo;
	MFPVideoQueue* frameQueue;
	MFPAudioQueue* audioQueue;
};
