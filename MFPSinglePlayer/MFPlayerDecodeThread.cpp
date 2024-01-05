#include "MFPlayerDecodeThread.h"

#include <imgproc.hpp>
#include <qcoreapplication.h>
#include <QTime>

MFPlayerDecodeThread::MFPlayerDecodeThread() { isStop = false; }

MFPlayerDecodeThread::MFPlayerDecodeThread(MFPFrameQueue<AVFrame>* frameQueue) {
	isStop = false;
	mFPVideo = new MFPVideo();
	this->frameQueue = frameQueue;

	mFPVideo->init();
	frameQueue->setFmt(mFPVideo->getFmt());
	frameQueue->setTotalTime(mFPVideo->getTotalTime());
	frameQueue->setFrameRate(mFPVideo->getFrameRate());
}

MFPlayerDecodeThread::~MFPlayerDecodeThread() {
	mFPVideo->freeResources();
	delete mFPVideo;
}

void MFPlayerDecodeThread::decode() {
	if (isStop||frameQueue->isQuit())
		return;
	frameQueue->decodeLock.lock();
	int temp = 1;
	//循环读取视频数据
	AVFrame* frame = nullptr;
	//查看上一个pts
	qint64 lPts = frameQueue->getLastPts();
	mFPVideo->jumpTo(lPts);
	while (!isStop && temp > 0 &&!frameQueue->isQuit()) {
		temp = mFPVideo->getNextFrame(frame);
		if (temp == 2) {
			if (frame->pts >= lPts && !isStop && !frameQueue->isQuit()) {
				frameQueue->safePut(*frame);
			}
			else
				av_frame_unref(frame);
		}
	}

	if (temp == 0) {
		//mFPVideo->jumpTo(0);
		frameQueue->frameIsEnd = true;
	}
	else { av_frame_unref(frame); }
	frameQueue->decodeLock.unlock();
	//return 0;
}

void MFPlayerDecodeThread::setFlag(bool flag) { isStop = flag; }

bool MFPlayerDecodeThread::getFlag() { return isStop; }
