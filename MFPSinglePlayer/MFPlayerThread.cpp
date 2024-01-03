#include "MFPlayerThread.h"
#include <qeventloop.h>
#include <QTime>
#include <qcoreapplication.h>
#include <qmath.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"

void MFPlayerThread::delay(int msec) {
	QTime dieTime = QTime::currentTime().addMSecs(msec);
	while (QTime::currentTime() < dieTime && !isStop)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MFPlayerThread::playNextFrame(AVFrame* frame)
{
	frameQueue->safeGet(*frame);
	cv::Mat mat = MFPVideo::AVFrameToMat(frame, frameQueue->getFmt());
	cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
	QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
	frameQueue->setLastPts(frame->pts);
	emit sendFrame(image.copy(image.rect()));
	emit sendProgress(frame->pts, frameQueue->getTotalTime());
}

void MFPlayerThread::continousPlayBack(AVFrame* frame)
{
	QTime t1;
	bool start = false;
	while (!(frameQueue->frameIsEnd && frameQueue->isEmpty()) && !isStop) {
		playNextFrame(frame);
		if (!start) {
			t1 = QTime::currentTime().addMSecs(-frame->pts);
		}
		start = true;
		delay(QTime::currentTime().msecsTo(t1.addMSecs(frame->pts)));
		av_frame_unref(frame);
	}
}

void MFPlayerThread::setFlag(bool flag) { isStop = flag; }

MFPlayerThread::MFPlayerThread(MFPFrameQueue<AVFrame>* frame) {
	isStop = false;
	frameQueue = frame;
	nowPts = 0;
}

void MFPlayerThread::onPlay(MFPlayerThreadState::statement sig) {
	if (isStop)
		return;
	frameQueue->playLock.lock();
	frameQueue->playEnd = false;
	emit stateChange(MFPlayerThreadState::PLAYING);
	AVFrame* frame = av_frame_alloc();

	switch (sig) {
	case MFPlayerThreadState::CONTINUEPLAY:
		continousPlayBack(frame);
		break;
	case MFPlayerThreadState::NEXFRAME:
		playNextFrame(frame);
		break;
		default:
			break;
	}

	av_frame_free(&frame);
	frameQueue->playLock.unlock();
	if(frameQueue->frameIsEnd && frameQueue->isEmpty()) {
		frameQueue->playEnd = true;
		//frameQueue->setLastPts(0);
	}
	emit stateChange(MFPlayerThreadState::PAUSE);
}
