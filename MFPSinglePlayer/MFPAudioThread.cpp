#include "MFPAudioThread.h"
#include <qeventloop.h>
#include <qcoreapplication.h>
#include "QMediaDevices"
#include "QThread"
void MFPAudioThread::delay(int msec) {
	QTime dieTime = QTime::currentTime().addMSecs(msec);
	while (QTime::currentTime() < dieTime && !isStop)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MFPAudioThread::playNextFrame(AVFrame* frame)
{
	audioQueue->safeGet(*frame);
	if (isStop|| !frame->data[0])
		return;
	QByteArray temp = MFPVideo::toQByteArray(*frame, audioQueue->getSwrctx());
	io->write(temp);
	audioQueue->setLastPts(frame->pts);
	
}

void MFPAudioThread::continousPlayBack(AVFrame* frame)
{
	bool start = false;
	int index = 0;
	const int delta = audioQueue->getFrameRate() / audioQueue->getSpeed();
	while (!(audioQueue->frameIsEnd && audioQueue->isEmpty()) && !isStop) {
		if (audioQueue->getSpeed() < 4) {
			playNextFrame(frame);
			if (isStop || !frame->data[0])
				continue;
			frame->pts /= audioQueue->getSpeed();
		}
		else {
			if (index == 0) {
				playNextFrame(frame);
				index = delta;
			}
			else {
				audioQueue->safeGet(*frame);
				index--;
			}
		}
		if (!start) {
			clock->setTime( QTime::currentTime().addMSecs(-frame->pts));
			clock->lock.unlock();
		}
		start = true;
		delay(QTime::currentTime().msecsTo(clock->getTime().addMSecs(frame->pts)));
		av_frame_unref(frame);
	}
}

void MFPAudioThread::clearAudioQueue()
{
	//由播放器负责清理
//audioQueue->playLock.lock();
	audioQueue->decodeLock.lock();
	if (!audioQueue->isEmpty()) {
		AVFrame* frame = av_frame_alloc();
		while (!audioQueue->isEmpty()) {
			*frame = audioQueue->front();
			av_frame_unref(frame);
			audioQueue->pop();
		}
		av_frame_free(&frame);
	}
	audioQueue->initQueue();
	audioQueue->decodeLock.unlock();
	//audioQueue->playLock.unlock();
}

MFPAudioThread::MFPAudioThread(MFPAudioQueue* audioQueue,MFPSTDClock* clock) {
	this->audioQueue = audioQueue;
	this->clock = clock;
	QAudioFormat fmt;
	fmt.setSampleRate(44100);
	fmt.setChannelCount(2);
	fmt.setSampleFormat(QAudioFormat::Int16);
	audioSink = new QAudioSink(fmt);
	io=audioSink->start();
}

MFPAudioThread::~MFPAudioThread()
{
	audioSink->stop();
	clearAudioQueue();
	delete audioSink;
}

void MFPAudioThread::setFlag(bool flag)
{
	isStop = flag;
}

void MFPAudioThread::onPlay(MFPlayerThreadState::statement sig) {
	if (isStop)
		return;
	audioQueue->audioLock.lock();
	AVFrame* frame = av_frame_alloc();

	switch (sig) {
	case MFPlayerThreadState::CONTINUEPLAY:
		continousPlayBack(frame);
		break;
	case MFPlayerThreadState::NEXTFRAME:
		playNextFrame(frame);
		av_frame_unref(frame);
		break;
	default:
		break;
	}
	av_frame_free(&frame);
	audioQueue->forcePutOut();
	clearAudioQueue();
	audioQueue->audioLock.unlock();
}
