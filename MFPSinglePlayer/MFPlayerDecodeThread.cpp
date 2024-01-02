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
	clearFrameQueue();
	delete mFPVideo;
}

void MFPlayerDecodeThread::decode() {
	if (isStop)
		return ;
	frameQueue->decodeLock.lock();
	int temp=1;
	//循环读取视频数据
	AVFrame* frame = nullptr;
	int i = 0;
	if (!mFPVideo->isParse()) {
		mFPVideo->init();
	}
	frameQueue->init();
	frameQueue->setFmt(mFPVideo->getFmt());
	frameQueue->setTotalTime(mFPVideo->getTotalTime());

	while (!isStop && temp>0) {
		
		temp = mFPVideo->getNextFrame(frame);
		if (temp == 2 && !isStop) {
			frameQueue->safePut(*frame);
		}
	}
	
	if (temp == 0) {
		mFPVideo->jumpTo(0);
		frameQueue->frameIsEnd = true;
	}else {
		av_frame_unref(frame);
	}
	frameQueue->decodeLock.unlock();
	//return 0;
}

void MFPlayerDecodeThread::setFlag(bool flag) { isStop = flag; }

bool MFPlayerDecodeThread::getFlag()
{
	return isStop;
}

void MFPlayerDecodeThread::onControlProgress(int msec)
{
	frameQueue->forceOut();
	clearFrameQueue();
	mFPVideo->jumpTo(msec);
	//return 0;
}

void MFPlayerDecodeThread::clearFrameQueue()
{
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
