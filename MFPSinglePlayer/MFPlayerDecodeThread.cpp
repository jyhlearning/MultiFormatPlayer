#include "MFPlayerDecodeThread.h"

#include <imgproc.hpp>
#include <qcoreapplication.h>
#include <QTime>

MFPlayerDecodeThread::MFPlayerDecodeThread() { isStop = false; }

MFPlayerDecodeThread::MFPlayerDecodeThread(MFPFrameQueue<AVFrame>* frameQueue) {
	isStop = false;
	mFPVideo = new MFPVideo();
	this->frameQueue = frameQueue;
}

MFPlayerDecodeThread::~MFPlayerDecodeThread() {
	mFPVideo->freeResources();
	if (!frameQueue->isEmpty()) {
		AVFrame* frame = av_frame_alloc();
		while (!frameQueue->isEmpty()) {
			*frame = frameQueue->front();
			av_frame_unref(frame);
		}
		av_frame_free(&frame);
	}
	delete mFPVideo;
}

int MFPlayerDecodeThread::decode() {
	if (isStop)
		return 0;
	int temp;
	//循环读取视频数据
	AVFrame* frame = nullptr;
	int i = 0;
	if (!mFPVideo->isParse() && frameQueue->playEnd) {
		frameQueue->init();
		mFPVideo->init();
		frameQueue->setFmt(mFPVideo->getFmt());
		if (!frameQueue->isEmpty()) {
			AVFrame* frame = av_frame_alloc();
			while (!frameQueue->isEmpty()) {
				*frame = frameQueue->front();
				av_frame_unref(frame);
			}
			av_frame_free(&frame);
		}
	}
	while (!isStop && !frameQueue->frameIsEnd) {
		temp = mFPVideo->getNextFrame(frame);
		if (temp == 2) {
			frameQueue->safePut(*frame);
			cv::Mat mat = MFPVideo::AVFrameToMat(frame, frameQueue->getFmt());
			cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
			QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		}
		else if (temp <= 0)
			break;
	}
	if (temp == 0) {
		frameQueue->frameIsEnd = true;
		mFPVideo->freeResources();
	}
	return 0;
}

void MFPlayerDecodeThread::setFlag(bool flag) { isStop = flag; }
