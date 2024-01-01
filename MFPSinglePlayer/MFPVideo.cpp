#include "MFPVideo.h"

int MFPVideo::init()
{
    //创建AVFormatContext
    pFormatCtx = avformat_alloc_context();
    //初始化pFormatCtx
    if (avformat_open_input(&pFormatCtx, videoPath, NULL, NULL) != 0) {
        qDebug("avformat_open_input err.");
        return -1;
    }

    //获取音视频流数据信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        avformat_close_input(&pFormatCtx);
        qDebug("avformat_find_stream_info err.");
        return -2;
    }

    //找到视频流的索引
	for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            streamIndex = i;
            isVideo = 0;
            break;
        }
    }

    //没有视频流就退出
    if (isVideo == -1) {
        avformat_close_input(&pFormatCtx);
        qDebug("nb_streams err.");
        return -3;
    }

    //获取视频流编码
    pAVctx = avcodec_alloc_context3(nullptr);

    //获取视频帧数
    frameRate = pFormatCtx->streams[streamIndex]->avg_frame_rate.num / pFormatCtx->streams[streamIndex]->avg_frame_rate.den;

    //查找解码器
    avcodec_parameters_to_context(pAVctx, pFormatCtx->streams[streamIndex]->codecpar);
    pCodec = avcodec_find_decoder(pAVctx->codec_id);
    if (pCodec == nullptr) {
        avcodec_close(pAVctx);
        avformat_close_input(&pFormatCtx);
        qDebug("avcodec_find_decoder err.");
        return -4;
    }

    //初始化pAVctx
    if (avcodec_open2(pAVctx, pCodec, NULL) < 0) {
        avcodec_close(pAVctx);
        avformat_close_input(&pFormatCtx);
        qDebug("avcodec_open2 err.");
        return -5;
    }

    //初始化pAVpkt
    pAVpkt = av_packet_alloc();

    //初始化数据帧空间
    pAVframe = av_frame_alloc();

    totalTime = pFormatCtx->streams[streamIndex]->duration * pFormatCtx->streams[streamIndex]->time_base.num / pFormatCtx->streams[streamIndex]->time_base.den;

    parse = true;
    hasFree = false;
    return 0;
}

MFPVideo::MFPVideo(){
    videoPath = "C:/Users/jiangyuhao/Videos/Overwolf/Valorant Tracker/VALORANT/VALORANT_07-29-2023_14-20-52-932/VALORANT 07-29-2023 14-36-48-992.mp4";
    isVideo = -1;
    streamIndex = 0;
    totalFrame = 0;
    parse = false;
    hasFree = true;
    frameRate = 30;
}

MFPVideo::~MFPVideo()
{
    freeResources();
}

void MFPVideo::freeResources(){
    if (!hasFree) {
    	av_frame_free(&pAVframe);
        avcodec_close(pAVctx);
        avformat_close_input(&pFormatCtx);
        parse = false;
        hasFree = true;
    }
}

qreal MFPVideo::rationalToDouble(const AVRational* rational)
{
    const qreal rate = (rational->den == 0) ? 0 : (qreal(rational->num) / rational->den);
    return rate;
}


int MFPVideo::getFrameRate() const
{
    return frameRate;
}

bool MFPVideo::isParse() const
{
    return parse;
}

int MFPVideo::getNextFrame(AVFrame* &frame){
    bool flag = true;

    if(av_read_frame(pFormatCtx, pAVpkt) >= 0){//读取一帧未解码的数据
        flag = false;
        //如果是视频数据
        if (pAVpkt->stream_index == (int)streamIndex){
            //解码一帧视频数据
        	pAVpkt->pts = qRound64(pAVpkt->pts * (1000 * rationalToDouble(&pFormatCtx->streams[(int)streamIndex]->time_base)));
        	pAVpkt->dts = qRound64(pAVpkt->dts * (1000 * rationalToDouble(&pFormatCtx->streams[(int)streamIndex]->time_base)));
            if (avcodec_send_packet(pAVctx, pAVpkt) == 0){
                // 一个avPacket可能包含多帧数据，所以需要使用while循环一直读取
                while(avcodec_receive_frame(pAVctx, pAVframe) == 0) {
	                    AVFrame* dst = av_frame_alloc();
	                    av_frame_move_ref(dst,pAVframe);
                		queue.push_back(dst);
                }
            }else{
                qDebug("Decode Error.\n");
                return -1;
            }
        }
		av_packet_unref(pAVpkt);
    }else {
        avcodec_send_packet(pAVctx, pAVpkt);
    }
    if (!queue.isEmpty()) {
		frame = queue.front();
        queue.pop_front();
    	totalFrame++;
        return 2;//正常帧
    }else if(flag) {
        jumpTo(0);
        return 0;//放完了
    }
    return 1;//空帧

}

int MFPVideo::jumpTo(qint64 sec) {
    int ret = av_seek_frame(pFormatCtx, streamIndex, sec * pFormatCtx->streams[streamIndex]->time_base.den/pFormatCtx->streams[streamIndex]->time_base.num, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(pAVctx);
    return ret;
}


AVPixelFormat MFPVideo::getFmt() const{
    return pAVctx->pix_fmt;
}


QImage MFPVideo::toQImage(const AVFrame& frame) {
	return QImage((uchar*)frame.data[0], frame.width, frame.height, QImage::Format_RGBA8888);
}
cv::Mat MFPVideo::AVFrameToMat(const AVFrame* frame,const AVPixelFormat fmt)
{
    const int image_width = frame->width;
    const int image_height = frame->height;

    cv::Mat resMat(image_height, image_width, CV_8UC3);
    int cvLinesizes[1];
    cvLinesizes[0] = resMat.step1();

    SwsContext* avFrameToOpenCVBGRSwsContext = sws_getContext(
        image_width,
        image_height,
        fmt,
        image_width,
        image_height,
        AVPixelFormat::AV_PIX_FMT_BGR24,
        SWS_FAST_BILINEAR,
        nullptr, nullptr, nullptr
    );

    sws_scale(avFrameToOpenCVBGRSwsContext,
        frame->data,
        frame->linesize,
        0,
        image_height,
        &resMat.data,
        cvLinesizes);

    if (avFrameToOpenCVBGRSwsContext != nullptr)
    {
        sws_freeContext(avFrameToOpenCVBGRSwsContext);
        avFrameToOpenCVBGRSwsContext = nullptr;
    }

    return resMat;
}


