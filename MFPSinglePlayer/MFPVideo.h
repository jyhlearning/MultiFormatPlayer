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
static enum AVPixelFormat g_pixelFormat;
class MFPVideo {
private:
	bool parse;
	bool hwFlag;
	const char* videoPath;
	int videoIndex, audioIndex;
	const AVCodec *pCodec, *aCodec;
	AVPacket* pAVpkt;
	AVCodecContext *pAVctx, *aAVctx;
	AVFrame* pAVframe;
	AVFormatContext* pFormatCtx;
	QQueue<AVFrame*> pQueue, aQueue;
	qint64 totalTime;
	//SwsContext* avFrameToOpenCVBGRSwsContext;
	
	QList<int> hwDeviceTypes;
	AVBufferRef* hwBufferRef;
	void initHWDecoder(const AVCodec* codec, AVCodecContext *ctx);
public:
	MFPVideo();
	~MFPVideo();
	AVCodecContext *getVideoCtx() const;
	AVCodecContext* getAudioCtx() const;
	AVStream* getVideoInStream() const;
	AVStream* getAudioInStream() const;
	AVSampleFormat getSampleFmt() const;
	int getSampleRate() const;
	int getChannels() const;
	qint64 getChannelsLayout() const;
	std::pair<int, int> getResolution() const;

	int init(const QString& url);
	int getNextInfo(AVFrame* &frame,int option=0);
	int readFrame(int index, int option,AVCodecContext* ctx, QQueue<AVFrame*> &queue) const;
	int jumpTo(qint64 usec);
	int getFrameRate() const;
	qint64 getTotalTime() const;
	bool isParse() const;
	void freeResources();
	void setVideoPath(const QString &path);
	void setHwFlag(bool flag);
	static QByteArray toQByteArray(const AVFrame* frame, SwrContext* swr_ctx);
	static QImage toQImage(const AVFrame* frame,SwsContext* avFrameToQImageSwsContext);
	static cv::Mat AVFrameToMat(const AVFrame* frame,SwsContext * avFrameToOpenCVBGRSwsContext);
	static qreal rationalToDouble(const AVRational* rational);
	static qint64 toMsec(const qint64 msec, const AVRational* rational);
};
