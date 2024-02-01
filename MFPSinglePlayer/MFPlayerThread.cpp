#include "MFPlayerThread.h"
#include <qeventloop.h>
#include <QTimer>
#include <qcoreapplication.h>
#include "QThread"

int temp = 0;
int MFPlayerThread::playNextFrame(AVFrame* &frame) {
	if (isStop || frameQueue->safeGet(frame)==-1)
		return -1;
	/*cv::Mat mat = MFPVideo::AVFrameToMat(frame, frameQueue->getSwsctx());
	cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
	QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);*/
	clock->setLastPts(frame->pts);
	emit sendFrame(MFPVideo::toQImage(frame, initSwsctx(frame)));
	emit sendProgress(frame->pts);
	return 0;
}

void MFPlayerThread::continousPlayBack() {
	int index = 0;
	AVFrame* frame = nullptr;
	const int delta = frameQueue->getFrameRate() / frameQueue->getSpeed();
	while (!(frameQueue->getIsEnd() && frameQueue->isEmpty()) && !isStop) {
		if (frameQueue->getSpeed() < 4) {
			if (!isStop && !playNextFrame(frame))
				frame->pts /= frameQueue->getSpeed();
		}
		else {
			if (index == 0) {
				playNextFrame(frame);
				index = delta;
			}
			else {
				frameQueue->safeGet(frame);
				index--;
			}
		}
		const qint64 pts = frame ? frame->pts : clock->getLastPts();
		clock->setStartTime(pts);
		MFPSTDClock::delay(clock->getDelayTime(pts));
		if (frame && frame->data[0])
			av_frame_free(&frame);
	}
}

SwsContext* MFPlayerThread::initSwsctx(AVFrame* frame)
{
	if (!avFrameToQImageSwsContext) {
		avFrameToQImageSwsContext = sws_getContext(
			frame->width,
			frame->height,
			(AVPixelFormat)frame->format,
			frame->width,
			frame->height,
			AVPixelFormat::AV_PIX_FMT_RGBA,
			SWS_FAST_BILINEAR,
			nullptr, nullptr, nullptr
		);
	}
	return avFrameToQImageSwsContext;
}

void MFPlayerThread::setFlag(bool flag) { isStop = flag; }

void MFPlayerThread::init()
{
	if (avFrameToQImageSwsContext) {
		sws_freeContext(avFrameToQImageSwsContext);
		avFrameToQImageSwsContext = nullptr;
	}
}

MFPlayerThread::MFPlayerThread(MFPFrameQueue* frame,MFPSTDClock* clock) {
	isStop = false;
	frameQueue = frame;
	this->clock = clock;
	avFrameToQImageSwsContext = nullptr;
}

MFPlayerThread::~MFPlayerThread() {
	if(avFrameToQImageSwsContext)
		sws_freeContext(avFrameToQImageSwsContext);
}

void MFPlayerThread::onPlay(MFPlayerThreadState::statement sig) {
	//frameQueue->playLock.lock();
	if (!isStop) {
		emit stateChange(MFPlayerThreadState::PLAYING);
		AVFrame* frame = nullptr;

		switch (sig) {
		case MFPlayerThreadState::CONTINUEPLAY:
			continousPlayBack();
			break;
		case MFPlayerThreadState::NEXTFRAME:
			playNextFrame(frame);
			av_frame_free(&frame);
			break;
		default:
			break;
		}
		frameQueue->forcePutOut();
		//说明播放完了
		if (!isStop && frameQueue->getIsEnd()) {
			frameQueue->setPlayEnd(true);
		}
		emit release();
		emit stateChange(MFPlayerThreadState::PAUSE);
	}
	//frameQueue->playLock.unlock();
}
