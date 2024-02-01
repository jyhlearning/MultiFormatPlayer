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
	QAudioFormat fmt;
	QIODevice* io;
	QAudioSink* audioSink;
	MFPAudioQueue *audioQueue;
	MFPSTDClock* clock;
	SwrContext* swr_ctx;
	int playNextFrame(AVFrame* &frame);
	void continousPlayBack();
	SwrContext* initSwrctx(AVFrame* frame,AVSampleFormat fmt);
public:
	MFPAudioThread(MFPAudioQueue* audioQueue,MFPSTDClock *clock);
	~MFPAudioThread();
	void setFlag(bool flag);
	void init();
public slots:
	void onPlay(MFPlayerThreadState::statement sig);
	void onVolume(int v) const;
signals:
	void release();
};

