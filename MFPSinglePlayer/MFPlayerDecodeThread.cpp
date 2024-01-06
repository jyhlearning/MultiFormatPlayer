#include "MFPlayerDecodeThread.h"
#include <qcoreapplication.h>

MFPlayerDecodeThread::MFPlayerDecodeThread() { isStop = false; }

MFPlayerDecodeThread::MFPlayerDecodeThread(MFPFrameQueue* frameQueue, MFPAudioQueue* audioQueue,MFPVideo* mFPVideo) {
	isStop = false;
	this->frameQueue = frameQueue;
	this->mFPVideo = mFPVideo;
	this->audioQueue = audioQueue;
}

MFPlayerDecodeThread::~MFPlayerDecodeThread() {
}

void MFPlayerDecodeThread::decode() {
	if (frameQueue->isQuit()||audioQueue->isQuit())
		return;
	frameQueue->decodeLock.lock();
	audioQueue->decodeLock.lock();
	int temp = 1;
	//循环读取视频数据
	AVFrame* frame = nullptr;
	qint64 lPts = frameQueue->getLastPts();
	while (temp > 0 && !(frameQueue->isQuit()&&audioQueue->isQuit())) {
		temp = mFPVideo->getNextInfo(frame);
		if (temp == 2||temp==3) {
			if (frame->pts >= lPts && !(frameQueue->isQuit() && audioQueue->isQuit())) {
				if (temp == 2 && !frameQueue->isQuit())
					frameQueue->safePut(*frame);
				else if (temp == 3 && !audioQueue->isQuit())
					audioQueue->safePut(*frame);
				else
					av_frame_unref(frame);
			}
			else
				av_frame_unref(frame);
		}
	}

	if (temp == 0) {
		frameQueue->frameIsEnd = true;
		audioQueue->frameIsEnd = true;
		frameQueue->forceGetOut();
		audioQueue->forceGetOut();
	}
	else { av_frame_unref(frame); }
	frameQueue->decodeLock.unlock();
	audioQueue->decodeLock.unlock();
	//return 0;
}

void MFPlayerDecodeThread::setFlag(bool flag) { isStop = flag; }

bool MFPlayerDecodeThread::getFlag() { return isStop; }
