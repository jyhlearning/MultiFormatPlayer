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

int MFPAudioThread::playNextFrame(AVFrame* &frame)
{
	if (isStop || audioQueue->safeGet(frame) == -1)
		return -1;
	io->write(MFPVideo::toQByteArray(frame, audioQueue->getSwrctx()));
	audioQueue->setLastPts(frame->pts);
	return 0;
}

void MFPAudioThread::continousPlayBack()
{
	bool start = false;
	int index = 0;
	AVFrame* frame;
	const int delta = audioQueue->getFrameRate() / audioQueue->getSpeed();
	while (!(audioQueue->frameIsEnd && audioQueue->isEmpty()) && !isStop) {
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
		const qint64 pts= frame?frame->pts:0;
		if (!start) {
			clock->setTime( QTime::currentTime().addMSecs(-pts));
			clock->lock.unlock();
			start = true;
		}
		delay(QTime::currentTime().msecsTo(clock->getTime().addMSecs(pts)));
		if(frame)
			av_frame_free(&frame);
	}
}

void MFPAudioThread::clearAudioQueue()
{
	//由播放器负责清理
//audioQueue->playLock.lock();
	audioQueue->decodeLock.lock();
	if (!audioQueue->isEmpty()) {
		while (!audioQueue->isEmpty()) {
			AVFrame* frame = audioQueue->front();
			av_frame_free(&frame);
			audioQueue->pop();
		}
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
	fmt.setChannelCount(audioQueue->getChannels());
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

void MFPAudioThread::onVolume(int v) const
{
	audioSink->setVolume(v*1.0/100);
}

void MFPAudioThread::onPlay(MFPlayerThreadState::statement sig) {
	if (isStop)
		return;
	audioQueue->audioLock.lock();

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
	audioQueue->forcePutOut();
	clearAudioQueue();
	audioQueue->audioLock.unlock();
}
