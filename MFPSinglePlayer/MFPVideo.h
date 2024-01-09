#pragma once
#include "QImage"
#include <QQueue>
#include <core/mat.hpp>
#include <QByteArray>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

class MFPVideo {
private:
	bool parse;
	bool hasFree;
	const char* videoPath;
	unsigned char* buf;
	int videoIndex, audioIndex;
	const AVCodec *pCodec, *aCodec;
	AVPacket* pAVpkt;
	AVCodecContext *pAVctx, *aAVctx;
	AVFrame* pAVframe;
	AVFormatContext* pFormatCtx;
	QQueue<AVFrame*> pQueue, aQueue;
	qint64 totalTime;
	SwrContext* swr_ctx;
	SwsContext* avFrameToOpenCVBGRSwsContext;
public:
	MFPVideo();
	~MFPVideo();
	SwsContext* getSwsctx() const;
	SwrContext* getSwrctx() const;
	int getSampleRate() const;
	int getChannels() const;
	std::pair<int, int> getResolution() const;

	int init();
	int getNextInfo(AVFrame* &frame);
	int jumpTo(qint64 usec);
	int getFrameRate() const;
	qint64 getTotalTime() const;
	bool isParse() const;
	void freeResources();
	static QByteArray toQByteArray(const AVFrame* frame, SwrContext* swr_ctx);
	static QImage toQImage(const AVFrame& frame);
	static cv::Mat AVFrameToMat(const AVFrame* frame,SwsContext *fmt);
	static qreal rationalToDouble(const AVRational* rational);
};
