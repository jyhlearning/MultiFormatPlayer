#include "MFPAudioThread.h"
#include "QMediaDevices"
#include "QThread"
#include "QTimer"

SwrContext* MFPAudioThread::initSwrctx(AVFrame* frame, AVSampleFormat fmt) {
	if (!swr_ctx) {
		swr_ctx = swr_alloc();
		av_opt_set_int(swr_ctx, "in_channel_layout", frame->channel_layout, 0);
		av_opt_set_int(swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
		av_opt_set_int(swr_ctx, "in_sample_rate", frame->sample_rate, 0);
		av_opt_set_int(swr_ctx, "out_sample_rate", 44100, 0);
		av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", fmt, 0);
		av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
		swr_init(swr_ctx);
	}
	return swr_ctx;
}

void MFPAudioThread::io(AVFrame* frame) {
	ioDevice->write(MFPVideo::toQByteArray(frame, initSwrctx(frame, audioQueue->getSampleFmt())));
}

MFPAudioThread::MFPAudioThread(MFPAudioQueue* audioQueue, MFPSTDClock* clock): MFPlayBase(audioQueue, clock) {
	fmt.setSampleRate(44100);
	fmt.setChannelCount(audioQueue->getChannels());
	fmt.setSampleFormat(QAudioFormat::Int16);
	this->audioQueue = audioQueue;
	audioSink = new QAudioSink(fmt);
	ioDevice = audioSink->start();
	swr_ctx = nullptr;
}

MFPAudioThread::~MFPAudioThread() {
	audioSink->stop();
	delete audioSink;
	if (swr_ctx)
		swr_free(&swr_ctx);
}

void MFPAudioThread::init() {
	if (swr_ctx) {
		swr_free(&swr_ctx);
		swr_ctx = nullptr;
	}
}

void MFPAudioThread::onVolume(int v) const { audioSink->setVolume(v * 1.0 / 100); }
