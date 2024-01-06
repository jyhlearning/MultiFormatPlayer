#include "MFPlayerThread.h"
#include <qeventloop.h>
#include <QTime>
#include <qcoreapplication.h>
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
	if (isStop || !frame->data[0])
		return;
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
			if (isStop || !frame->data[0])
				continue;
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



void MFPlayerThread::clearFrameQueue() {
	//由播放器负责清理
	//frameQueue->playLock.lock();
	frameQueue->decodeLock.lock();
	if (!frameQueue->isEmpty()) {
		AVFrame* frame = av_frame_alloc();
		while (!frameQueue->isEmpty()) {
			*frame = frameQueue->front();
			av_frame_unref(frame);
			frameQueue->pop();
		}
		av_frame_free(&frame);
	}
	frameQueue->initQueue();
	frameQueue->decodeLock.unlock();
	//frameQueue->playLock.unlock();
}

void MFPlayerThread::setFlag(bool flag) { isStop = flag; }

MFPlayerThread::MFPlayerThread(MFPFrameQueue* frame) {
	isStop = false;
	frameQueue = frame;
}

MFPlayerThread::~MFPlayerThread() {
	clearFrameQueue();
}

void MFPlayerThread::onPlay(MFPlayerThreadState::statement sig) {
	if (isStop)
		return;
	frameQueue->playLock.lock();
	emit stateChange(MFPlayerThreadState::PLAYING);
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
	frameQueue->forcePutOut();
	clearFrameQueue();
	frameQueue->playLock.unlock();
	emit stateChange(MFPlayerThreadState::PAUSE);
}
