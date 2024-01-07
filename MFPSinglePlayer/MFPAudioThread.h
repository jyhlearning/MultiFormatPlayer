#pragma once
#include "MFPAudioQueue.h"
#include "MFPlayerThread.h"
#include "MFPSTDClock.h"
#include "QAudioSink"
#include "QObject"
class MFPAudioThread:public QObject
{
	Q_OBJECT
private:
	bool isStop;
	QAudioSink *audioSink;
	QIODevice* io;
	MFPAudioQueue *audioQueue;
	MFPSTDClock* clock;
	void delay(int msec);
	int playNextFrame(AVFrame* &frame);
	void continousPlayBack();
	void clearAudioQueue();
public:
	MFPAudioThread(MFPAudioQueue* audioQueue,MFPSTDClock *clock);
	~MFPAudioThread();
	void setFlag(bool flag);
public slots:
	void onPlay(MFPlayerThreadState::statement sig);
	void onVolume(int v) const;
};

