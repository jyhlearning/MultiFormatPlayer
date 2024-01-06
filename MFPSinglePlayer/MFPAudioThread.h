#pragma once
#include "MFPAudioQueue.h"
#include "MFPlayerThread.h"
#include "MFPAudioDecodeThread.h"
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
	void delay(int msec);
	void playNextFrame(AVFrame* frame);
	void continousPlayBack(AVFrame* frame);
	void clearAudioQueue();
public:
	MFPAudioThread(MFPAudioQueue* audioQueue);
	~MFPAudioThread();
	void setFlag(bool flag);
public slots:
	void onPlay(MFPlayerThreadState::statement sig);
};

