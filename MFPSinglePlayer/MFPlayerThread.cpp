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

void MFPlayerThread::setFlag(bool flag) { isStop = flag; }

MFPlayerThread::MFPlayerThread(MFPFrameQueue<AVFrame>* frame) {
	isStop = false;
	frameQueue = frame;
}

void MFPlayerThread::onPlay() {
	if (isStop)
		return;
	int delayTime = 0;
	bool start = false;
	emit stateChange(PLAYING);
	AVFrame *frame = av_frame_alloc();
	int delta = 0;
	QTime t1;
	while (!(frameQueue->frameIsEnd&&frameQueue->isEmpty()) && !isStop) {
		frameQueue->safeGet(*frame);
		cv::Mat mat=MFPVideo::AVFrameToMat(frame,frameQueue->getFmt());

		cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
		QImage image(mat.data,mat.cols,mat.rows,mat.step,QImage::Format_RGB888);

		emit sendFrame(image.copy(image.rect()));
		if(!start) {
			frameQueue->playEnd = false;
			t1 = QTime::currentTime().addMSecs(-frame->pts);
		}
		QTime t2 = t1.addMSecs(frame->pts);
		delay(QTime::currentTime().msecsTo(t2));
		av_frame_unref(frame);
		start = true;
	}
	av_frame_free(&frame);
	if(frameQueue->frameIsEnd && frameQueue->isEmpty()) {
		frameQueue->playEnd = true;
	}
	emit stateChange(PAUSE);
}
