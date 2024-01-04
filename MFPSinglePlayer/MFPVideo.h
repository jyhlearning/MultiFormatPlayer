#pragma once
#include "QImage"
#include <QQueue>
#include <core/mat.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
}

class MFPVideo {
private:
	bool parse;
	bool hasFree;
	const char* videoPath;
	unsigned char* buf;
	int isVideo;
	int streamIndex;
	const AVCodec* pCodec;
	AVPacket* pAVpkt;
	AVCodecContext* pAVctx;
	AVFrame *pAVframe, *pAVframeRGB;
	AVFormatContext* pFormatCtx;
	struct SwsContext* pSwsCtx;
	QQueue<AVFrame*> queue;
	qint64 totalFrame;
	qint64 totalTime;

public:
	MFPVideo();
	~MFPVideo();
	AVPixelFormat getFmt() const;
	int init();
	int getNextFrame(AVFrame* & frame);
	int jumpTo(qint64 usec);
	int getFrameRate() const;
	qint64 getTotalTime() const;
	bool isParse() const;
	void freeResources();
	static QImage toQImage(const AVFrame& frame);
	static cv::Mat AVFrameToMat(const AVFrame* frame, const AVPixelFormat fmt);
	static qreal rationalToDouble(const AVRational* rational);
};
