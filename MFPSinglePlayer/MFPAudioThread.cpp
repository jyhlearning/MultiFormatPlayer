#include "MFPAudioThread.h"
#include <qeventloop.h>
#include <qcoreapplication.h>

#include "QMediaDevices"
#include "QThread"
#include "QTimer"

int MFPAudioThread::playNextFrame(AVFrame* & frame) {
	if (isStop || audioQueue->safeGet(frame) == -1)
		return -1;
	io->write(MFPVideo::toQByteArray(frame, initSwrctx(frame, audioQueue->getSampleFmt())));
	clock->setLastPts(frame->pts);
	return 0;
}

void MFPAudioThread::continousPlayBack() {
	qint64 index = 0;
	AVFrame* frame = nullptr;
	const qint64 delta = audioQueue->getFrameRate() / audioQueue->getSpeed();
	while (!(audioQueue->getIsEnd() && audioQueue->isEmpty()) && !isStop) {
		if (audioQueue->getSpeed() < 4) {
			if (!isStop && !playNextFrame(frame))
				frame->pts /= audioQueue->getSpeed();
		}
		else {
			if (index == 0) {
				playNextFrame(frame);
				index = delta;
			}
			else {
				audioQueue->safeGet(frame);
				index--;
			}
		}
		const qint64 pts = frame ? frame->pts : clock->getLastPts();
		clock->setStartTime(pts);
		MFPSTDClock::delay(clock->getDelayTime(pts));
		if (frame && frame->data[0])
			av_frame_free(&frame);
	}
}

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

MFPAudioThread::MFPAudioThread(MFPAudioQueue* audioQueue, MFPSTDClock* clock) {
	this->audioQueue = audioQueue;
	this->clock = clock;
	fmt.setSampleRate(44100);
	fmt.setChannelCount(audioQueue->getChannels());
	fmt.setSampleFormat(QAudioFormat::Int16);
	audioSink = new QAudioSink(fmt);
	io = audioSink->start();
	swr_ctx = nullptr;
}

MFPAudioThread::~MFPAudioThread() {
	audioSink->stop();
	delete audioSink;
	if (swr_ctx)
		swr_free(&swr_ctx);
}

void MFPAudioThread::setFlag(bool flag) { isStop = flag; }

void MFPAudioThread::init() {
	if (swr_ctx) {
		swr_free(&swr_ctx);
		swr_ctx = nullptr;
	}
}

void MFPAudioThread::onVolume(int v) const { audioSink->setVolume(v * 1.0 / 100); }

void MFPAudioThread::onPlay(MFPlayerThreadState::statement sig) {
	//audioQueue->audioLock.lock();
	if (!isStop) {
		AVFrame* frame = nullptr;
		switch (sig) {
		case MFPlayerThreadState::CONTINUEPLAY:
			continousPlayBack();
			break;
		case MFPlayerThreadState::NEXTFRAME:
			playNextFrame(frame);
			av_frame_free(&frame);
			break;
		default:
			break;
		}
		//说明播放完了
		if (!isStop && audioQueue->getIsEnd()) { audioQueue->setPlayEnd(true); }
		audioQueue->forcePutOut();
		emit release();
	}
	//audioQueue->audioLock.unlock();
}
