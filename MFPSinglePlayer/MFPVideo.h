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
class MFPVideo
{
private:
    bool parse;
    bool hasFree;
    int frameRate;
    const char* videoPath;
    unsigned char* buf;
    int isVideo;
    unsigned int streamIndex;
    const AVCodec* pCodec;
    AVPacket* pAVpkt;
    AVCodecContext* pAVctx;
    AVFrame* pAVframe, * pAVframeRGB;
    AVFormatContext* pFormatCtx;
    struct SwsContext* pSwsCtx;
    QQueue<AVFrame* > queue;
    qint64 totalFrame;
    static qreal rationalToDouble(const AVRational* rational);

public:
    MFPVideo();
    ~MFPVideo();
    int init();
    int getNextFrame(AVFrame* &frame);
    void freeResources();
    int getFrameRate() const;
    bool isParse() const;
    AVPixelFormat getFmt() const;
	static QImage toQImage(const AVFrame &frame);
    static cv::Mat AVFrameToMat(const AVFrame* frame,const AVPixelFormat fmt);
};

