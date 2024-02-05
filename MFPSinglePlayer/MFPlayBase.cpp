#include "MFPlayBase.h"

int MFPlayBase::playNextFrame(AVFrame*& frame)
{
	if (isStop || queue->safeGet(frame) == -1)
		return -1;
	io(frame);
	clock->setLastPts(frame->pts);
	emit sendProgress(frame->pts);
	return 0;
}

void MFPlayBase::continousPlayBack() {
	qint64 index = 0;
	AVFrame* frame = nullptr;
	const qint64 delta = queue->getFrameRate() / queue->getSpeed();
	while (!(queue->getIsEnd() && queue->isEmpty()) && !isStop) {
		if (queue->getSpeed() < 4) {
			if (!isStop && !playNextFrame(frame))
				frame->pts /= queue->getSpeed();
		}
		else {
			if (index == 0) {
				playNextFrame(frame);
				index = delta;
			}
			else {
				queue->safeGet(frame);
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

MFPlayBase::MFPlayBase(MFPDataBase<AVFrame*>* queue, MFPSTDClock* clock) {
	isStop = true;
	this->queue = queue;
	this->clock = clock;
}

void MFPlayBase::setFlag(bool flag) { isStop = flag; }

void MFPlayBase::onPlay(MFPlayState::statement sig) {
	if (!isStop) {
		emit stateChange(MFPlayState::PLAYING);
		AVFrame* frame = nullptr;
		switch (sig) {
		case MFPlayState::CONTINUEPLAY:
			continousPlayBack();
			break;
		case MFPlayState::NEXTFRAME:
			playNextFrame(frame);
			av_frame_free(&frame);
			break;
		default:
			break;
		}
		//说明播放完了
		if (!isStop && queue->getIsEnd()) { queue->setPlayEnd(true); }
		queue->forcePutOut();
		emit release();
		emit stateChange(MFPlayState::PAUSE);
	}
}
