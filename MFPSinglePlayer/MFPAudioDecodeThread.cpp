#include "MFPAudioDecodeThread.h"

MFPAudioDecodeThread::MFPAudioDecodeThread()
{
	isStop = false;
}

MFPAudioDecodeThread::MFPAudioDecodeThread(MFPAudioQueue* audioQueue, MFPVideo* mFPVideo) {
	this->audioQueue = audioQueue;
	this->mFPVideo = mFPVideo;
}

MFPAudioDecodeThread::~MFPAudioDecodeThread() {
}

void MFPAudioDecodeThread::setFlag(bool flag) { isStop = flag; }

bool MFPAudioDecodeThread::getFlag() { return isStop; }

void MFPAudioDecodeThread::decode() {
	if (isStop || audioQueue->isQuit())
		return;
	audioQueue->decodeLock.lock();
	int temp = 1;

	AVFrame* frame = nullptr;

	qint64 lPts = audioQueue->getLastPts();
	while (!isStop && temp > 0 && !audioQueue->isQuit()) {
		temp = mFPVideo->getNextInfo(frame);
		if (temp == 2) {
			if (frame->pts >= lPts && !isStop && !audioQueue->isQuit()) {
				audioQueue->safePut(*frame);
			}
			else
				av_frame_unref(frame);
		}
	}

	if (temp == 0) {
		audioQueue->frameIsEnd = true;
	}
	else { av_frame_unref(frame); }
	audioQueue->decodeLock.unlock();
}
