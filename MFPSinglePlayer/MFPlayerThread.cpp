#include "MFPlayerThread.h"
#include <qeventloop.h>
#include <QTime>
#include <qcoreapplication.h>
#include <qmath.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "QThread"

void MFPlayerThread::delay(int msec) {
	QTime dieTime = QTime::currentTime().addMSecs(msec);
	while (QTime::currentTime() < dieTime && !isStop)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MFPlayerThread::playNextFrame(AVFrame* frame) {
	frameQueue->safeGet(*frame);
	cv::Mat mat = MFPVideo::AVFrameToMat(frame, frameQueue->getFmt());
	cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
	QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
	frameQueue->setLastPts(frame->pts);
	emit sendFrame(image.copy(image.rect()));
	emit sendProgress(frame->pts);
}

void MFPlayerThread::continousPlayBack(AVFrame* frame) {
	QTime t1;
	bool start = false;
	int index = 0;
	const int delta = frameQueue->getFrameRate() / frameQueue->getSpeed();
	while (!(frameQueue->frameIsEnd && frameQueue->isEmpty()) && !isStop) {
		if (frameQueue->getSpeed() < 4) {
			playNextFrame(frame);
			frame->pts /= frameQueue->getSpeed();
		}
		else {
			if (index == 0) {
				playNextFrame(frame);
				index = delta;
			}
			else {
				frameQueue->safeGet(*frame);
				index--;
			}
		}
		if (!start) { t1 = QTime::currentTime().addMSecs(-frame->pts); }
		start = true;
		delay(QTime::currentTime().msecsTo(t1.addMSecs(frame->pts)));
		av_frame_unref(frame);
	}
}

void MFPlayerThread::startDecode() {
	mFPlayerDecodeThread->setFlag(false);
	emit startDecodeThread();
}

void MFPlayerThread::stopDecode() {
	//frameQueue->forceOut();
	mFPlayerDecodeThread->setFlag(true);
}

void MFPlayerThread::clearFrameQueue() {
	//由播放器负责清理
	frameQueue->decodeLock.lock();
	frameQueue->playLock.lock();
	if (!frameQueue->isEmpty()) {
		AVFrame* frame = av_frame_alloc();
		while (!frameQueue->isEmpty()) {
			*frame = frameQueue->front();
			av_frame_unref(frame);
			frameQueue->pop();
		}
		av_frame_free(&frame);
	}
	frameQueue->init();
	frameQueue->decodeLock.unlock();
	frameQueue->playLock.unlock();
}

void MFPlayerThread::setFlag(bool flag) { isStop = flag; }

MFPlayerThread::MFPlayerThread(MFPFrameQueue<AVFrame>* frame) {
	isStop = false;
	frameQueue = frame;
	nowPts = 0;
	mFPlayerDecodeThread = new MFPlayerDecodeThread(frameQueue);
	mFPlayerDecodeThread->moveToThread(new QThread(this));
	mFPlayerDecodeThread->thread()->start();
	connect(this, SIGNAL(startDecodeThread()), mFPlayerDecodeThread, SLOT(decode()));
}

MFPlayerThread::~MFPlayerThread() {
	stopDecode();
	clearFrameQueue();
	if (mFPlayerDecodeThread->thread()->isRunning()) {
		mFPlayerDecodeThread->thread()->quit();
		mFPlayerDecodeThread->thread()->wait();
	}
	delete mFPlayerDecodeThread;
}

void MFPlayerThread::onPlay(MFPlayerThreadState::statement sig) {
	if (isStop)
		return;
	frameQueue->playLock.lock();

	startDecode();
	emit stateChange(MFPlayerThreadState::PLAYING);
	AVFrame* frame = av_frame_alloc();

	switch (sig) {
	case MFPlayerThreadState::CONTINUEPLAY:
		continousPlayBack(frame);
		break;
	case MFPlayerThreadState::NEXTFRAME:
		playNextFrame(frame);
		break;
	default:
		break;
	}
	av_frame_free(&frame);
	stopDecode();
	frameQueue->playLock.unlock();

	clearFrameQueue();

	emit stateChange(MFPlayerThreadState::PAUSE);
}
