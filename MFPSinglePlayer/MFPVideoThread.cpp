#include "MFPVideoThread.h"

SwsContext* MFPVideoThread::initSwsctx(AVFrame* frame) {
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

void MFPVideoThread::io(AVFrame* frame) {
	emit sendFrame(MFPVideo::toQImage(frame, initSwsctx(frame)));
}

void MFPVideoThread::init() {
	if (avFrameToQImageSwsContext) {
		sws_freeContext(avFrameToQImageSwsContext);
		avFrameToQImageSwsContext = nullptr;
	}
}

MFPVideoThread::MFPVideoThread(MFPVideoQueue* frame, MFPSTDClock* clock): MFPlayBase(frame, clock) {
	frameQueue = frame;
	this->clock = clock;
	avFrameToQImageSwsContext = nullptr;
}

MFPVideoThread::~MFPVideoThread() {
	if (avFrameToQImageSwsContext)
		sws_freeContext(avFrameToQImageSwsContext);
}
