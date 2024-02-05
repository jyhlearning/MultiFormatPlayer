#pragma once
#include "MFPAudioQueue.h"
#include "MFPSTDClock.h"
#include "QAudioSink"
#include "MFPlayBase.h"
#include "MFPVideo.h"

class MFPAudioThread : public MFPlayBase {
	Q_OBJECT

private:
	QAudioFormat fmt;
	QIODevice* ioDevice;
	QAudioSink* audioSink;
	SwrContext* swr_ctx;
	MFPAudioQueue* audioQueue;
	SwrContext* initSwrctx(AVFrame* frame, AVSampleFormat fmt);
	void io(AVFrame* frame) override;

public:
	MFPAudioThread(MFPAudioQueue* audioQueue, MFPSTDClock* clock);
	~MFPAudioThread();
	void init() override;

public slots:
	void onVolume(int v) const;
};
