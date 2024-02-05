#include "MFPlayerDecodeThread.h"
#include <qcoreapplication.h>

MFPlayerDecodeThread::MFPlayerDecodeThread() { isStop = false; }

MFPlayerDecodeThread::MFPlayerDecodeThread(MFPVideoQueue* frameQueue, MFPAudioQueue* audioQueue, MFPVideo* mFPVideo) {
	isStop = false;
	this->frameQueue = frameQueue;
	this->mFPVideo = mFPVideo;
	this->audioQueue = audioQueue;
}

MFPlayerDecodeThread::~MFPlayerDecodeThread() {
}

void MFPlayerDecodeThread::decode(const int option, const qint64 lPts) {
	if (frameQueue->isQuit() || audioQueue->isQuit())
		return;
	frameQueue->decodeLock.lock();
	audioQueue->decodeLock.lock();
	int temp = 1;
	bool b1 = false, b2 = false, a1 = mFPVideo->getVideoCtx(), a2 = mFPVideo->getVideoCtx();
	//循环读取视频数据
	const qint64 delta = ceil(1000.0 / frameQueue->getFrameRate() / 2);
	while (temp > 0 && !(frameQueue->isQuit() && audioQueue->isQuit())) {
		AVFrame* frame = nullptr;
		temp = mFPVideo->getNextInfo(frame);
		if (temp == 2 || temp == 3) {
			if (frame->pts - lPts > delta)
				if (temp == 2)
					b1 = true;
				else if (temp == 3)
					b2 = true;
			if (((option == 0 && frame->pts >= lPts) || (option == 1 && abs(frame->pts - lPts) <= delta)) && !(
				frameQueue->isQuit() && audioQueue->isQuit())) {
				if (temp == 2 && !frameQueue->isQuit())
					frameQueue->safePut(frame);
				else if (temp == 3 && !audioQueue->isQuit())
					audioQueue->safePut(frame);
				else av_frame_free(&frame);
			}
			else
				av_frame_free(&frame);
			if (((a1 && a2 && b1 && b2) || (a1 && !a2 && b1) || (!a1 && a2 && b2)) && option == 1) {
				temp = -1;
				break;
			}
		}
	}
	//解码完成，通知播放器，如果播放器在等待数据，解除等待状态
	if (temp <= 0) {
		if (temp == 0) {
			frameQueue->setIsEnd(true);
			audioQueue->setIsEnd(true);
		}
		frameQueue->forceGetOut();
		audioQueue->forceGetOut();
	}
	mFPVideo->clearBuffer();
	frameQueue->decodeLock.unlock();
	audioQueue->decodeLock.unlock();
	//return 0;
}

void MFPlayerDecodeThread::setFlag(bool flag) { isStop = flag; }

bool MFPlayerDecodeThread::getFlag() { return isStop; }
