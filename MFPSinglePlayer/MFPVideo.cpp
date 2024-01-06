﻿#include "MFPVideo.h"

int MFPVideo::init() {
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
			videoIndex = i;
			break;
		}
	}

	//找到音频流索引
	for (int i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioIndex = i;
			break;
		}
	}

	//没有视频流或者音频流就退出
	if (videoIndex == -1 || audioIndex == -1) {
		avformat_close_input(&pFormatCtx);
		qDebug("nb_streams err.");
		return -3;
	}

	//获取视频流编码
	pAVctx = avcodec_alloc_context3(nullptr);
	aAVctx = avcodec_alloc_context3(nullptr);

	//查找解码器
	avcodec_parameters_to_context(pAVctx, pFormatCtx->streams[videoIndex]->codecpar);
	avcodec_parameters_to_context(aAVctx, pFormatCtx->streams[audioIndex]->codecpar);

	pCodec = avcodec_find_decoder(pAVctx->codec_id);
	aCodec = avcodec_find_decoder(aAVctx->codec_id);

	if (pCodec == nullptr || aCodec == nullptr) {
		avcodec_close(pAVctx);
		avformat_close_input(&pFormatCtx);
		qDebug("avcodec_find_decoder err.");
		return -4;
	}

	//初始化pAVctx,aAVctx
	if (avcodec_open2(pAVctx, pCodec, nullptr) < 0||avcodec_open2(aAVctx,aCodec,nullptr)<0) {
		avcodec_close(pAVctx);
		avformat_close_input(&pFormatCtx);
		qDebug("avcodec_open2 err.");
		return -5;
	}

	//初始化pAVpkt
	pAVpkt = av_packet_alloc();

	//初始化数据帧空间
	pAVframe = av_frame_alloc();


	// 初始化音频重采样上下文
	swr_ctx = swr_alloc();
	av_opt_set_int(swr_ctx, "in_channel_layout", aAVctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", aAVctx->sample_rate, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", 44100, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", aAVctx->sample_fmt, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	swr_init(swr_ctx);

	totalTime = 1000 * pFormatCtx->duration / AV_TIME_BASE;

	parse = true;
	hasFree = false;
	return 0;


}

MFPVideo::MFPVideo() {
	videoPath =
		"C:/Users/jiangyuhao/Videos/Overwolf/Valorant Tracker/VALORANT/VALORANT_07-29-2023_14-20-52-932/VALORANT 07-29-2023 14-36-48-992.mp4";
	videoIndex = -1;
	audioIndex = -1;
	parse = false;
	hasFree = true;
}

MFPVideo::~MFPVideo() { freeResources(); }

void MFPVideo::freeResources() {
	if (!hasFree) {
		swr_free(&swr_ctx);
		av_frame_free(&pAVframe);
		avcodec_close(pAVctx);
		avformat_close_input(&pFormatCtx);
		parse = false;
		hasFree = true;
	}
}

SwrContext* MFPVideo::getSwrctx() const
{
	return swr_ctx;
}

qreal MFPVideo::rationalToDouble(const AVRational* rational) {
	const qreal rate = (rational->den == 0) ? 0 : (qreal(rational->num) / rational->den);
	return rate;
}


int MFPVideo::getFrameRate() const {
	return pFormatCtx->streams[videoIndex]->avg_frame_rate.num / pFormatCtx->streams[videoIndex]->avg_frame_rate.
		den;
}

qint64 MFPVideo::getTotalTime() const { return totalTime; }
 

QByteArray MFPVideo::toQByteArray(const AVFrame &frame,SwrContext* swr_ctx)
{
	uint8_t* buffer;
	av_samples_alloc(&buffer, NULL, av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO),
		frame.nb_samples, AV_SAMPLE_FMT_S16, 0);
	swr_convert(swr_ctx, &buffer, frame.nb_samples, (const uint8_t**)frame.data, frame.nb_samples);
	QByteArray temp = QByteArray(reinterpret_cast<const char*>(buffer),
		av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO) * frame.nb_samples);
	av_freep(&buffer);
	return temp;
}


bool MFPVideo::isParse() const { return parse; }

int MFPVideo::getNextInfo(AVFrame* & frame) {
	bool flag1 = true, flag2 = true;
	if (av_read_frame(pFormatCtx, pAVpkt) >= 0) {
		//读取一帧未解码的数据
		flag1 = false;
		//如果是视频数据
		if (pAVpkt->stream_index == videoIndex) {
			//解码一帧视频数据
			pAVpkt->pts = qRound64(
				pAVpkt->pts * (1000 * rationalToDouble(&pFormatCtx->streams[videoIndex]->time_base)));
			pAVpkt->dts = qRound64(
				pAVpkt->dts * (1000 * rationalToDouble(&pFormatCtx->streams[videoIndex]->time_base)));
			if (avcodec_send_packet(pAVctx, pAVpkt) == 0) {
				// 一个avPacket可能包含多帧数据，所以需要使用while循环一直读取
				while (avcodec_receive_frame(pAVctx, pAVframe) == 0) {
					AVFrame* dst = av_frame_alloc();
					av_frame_move_ref(dst, pAVframe);
					pQueue.push_back(dst);
				}
			}
			else {
				qDebug("Decode Error.\n");
				av_packet_unref(pAVpkt);
				return -1;
			}
		}else if(pAVpkt->stream_index==audioIndex) {
			//解码一帧音频数据
			pAVpkt->pts = qRound64(
				pAVpkt->pts * (1000 * rationalToDouble(&pFormatCtx->streams[audioIndex]->time_base)));
			pAVpkt->dts = qRound64(
				pAVpkt->dts * (1000 * rationalToDouble(&pFormatCtx->streams[audioIndex]->time_base)));
			if (avcodec_send_packet(aAVctx, pAVpkt) == 0) {
				// 一个avPacket可能包含多帧数据，所以需要使用while循环一直读取
				while (avcodec_receive_frame(aAVctx, pAVframe) == 0) {
					AVFrame* dst = av_frame_alloc();
					av_frame_move_ref(dst, pAVframe);
					aQueue.push_back(dst);
				}
			}
			else {
				qDebug("Decode Error.\n");
				av_packet_unref(pAVpkt);
				return -1;
			}
		}
		av_packet_unref(pAVpkt);
	}
	else {
		//处理最后buffer中的最后几帧
		pAVpkt->data = nullptr;
		pAVpkt->size = 0;
		avcodec_send_packet(pAVctx, pAVpkt);
		while (avcodec_receive_frame(pAVctx, pAVframe) == 0) {
			flag2 = false;
			AVFrame* dst = av_frame_alloc();
			av_frame_move_ref(dst, pAVframe);
			pQueue.push_back(dst);
		}
	}
	if (!pQueue.isEmpty()) {
		frame = pQueue.front();
		pQueue.pop_front();
		return 2; //正常帧
	}else if(!aQueue.isEmpty()) {
		frame = aQueue.front();
		aQueue.pop_front();
		return 3; //正常帧
	}
	else if (flag1 && flag2) {
		return 0; //放完了
	}
	return 1; //空帧
}

int MFPVideo::jumpTo(qint64 msec) {
	//int ret = av_seek_frame(pFormatCtx, videoIndex,
	//                        av_rescale_q(msec, av_make_q(1, 1000),
	//							pFormatCtx->streams[videoIndex]->time_base), AVFMT_SEEK_TO_PTS);


	// 使用 avformat_seek_file 定位整个文件
	qint64 min_timestamp = pFormatCtx->start_time != AV_NOPTS_VALUE ? pFormatCtx->start_time : 0;
	qint64 max_timestamp = INT64_MAX;
	//往前推1s,避免找到的关键帧太大
	msec = msec - 1000 < 0 ? 0 : msec - 1000;

	int ret = avformat_seek_file(pFormatCtx, videoIndex, min_timestamp, av_rescale_q(msec, av_make_q(1, 1000),
		                             pFormatCtx->streams[videoIndex]->time_base), max_timestamp, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pAVctx);

	return ret;
}


AVPixelFormat MFPVideo::getFmt() const { return pAVctx->pix_fmt; }


QImage MFPVideo::toQImage(const AVFrame& frame) {
	return QImage((uchar*)frame.data[0], frame.width, frame.height, QImage::Format_RGBA8888);
}

cv::Mat MFPVideo::AVFrameToMat(const AVFrame* frame, const AVPixelFormat fmt) {
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

	if (avFrameToOpenCVBGRSwsContext != nullptr) {
		sws_freeContext(avFrameToOpenCVBGRSwsContext);
		avFrameToOpenCVBGRSwsContext = nullptr;
	}

	return resMat;
}
