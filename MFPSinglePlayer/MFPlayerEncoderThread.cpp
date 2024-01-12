#include "MFPlayerEncoderThread.h"

void MFPlayerEncoderThread::init() {
	isStop = false;
	s.outWidth = mFPVideo->getVideoCtx()->width;
	s.outHeight = mFPVideo->getVideoCtx()->height;
	s.videoBitRate = mFPVideo->getVideoCtx()->bit_rate;
	s.audioBitRate = mFPVideo->getAudioCtx()->bit_rate;
	s.startPts = 0;
	s.endPts = mFPVideo->getTotalTime();
	s.closeAudio = s.closeVideo = false;
}

int MFPlayerEncoderThread::writeFrame(AVPacket* packet, AVFrame* frame, AVStream* inStream, AVStream* outStream,
                                      AVCodecContext* context, AVFormatContext* fmtCtx) {
	int ret = avcodec_send_frame(context, frame);
	while (avcodec_receive_packet(context, packet) == 0 && !isStop) {
		// 写入输出文件
		packet->stream_index = outStream->index;
		packet->pts = av_rescale_q_rnd(packet->pts, inStream->time_base, outStream->time_base,
		                               (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		packet->dts = av_rescale_q_rnd(packet->dts, inStream->time_base, outStream->time_base,
		                               (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		packet->duration = av_rescale_q(packet->duration, inStream->time_base, outStream->time_base);
		packet->pos = -1;
		av_interleaved_write_frame(fmtCtx, packet);
		av_packet_unref(packet);
	}
	av_frame_free(&frame);
	return 0;
}

void MFPlayerEncoderThread::setFlag(bool flag) { isStop = flag; }

MFPlayerEncoderThread::MFPlayerEncoderThread(MFPVideo* mFPVideo) {
	this->mFPVideo = mFPVideo;
	init();
}

void MFPlayerEncoderThread::setProfile(const settings& s) { this->s = s; }

settings MFPlayerEncoderThread::exportDefaultProfile() { return s; }

int MFPlayerEncoderThread::encode() {
	if (isStop)return 0;
	// 1. 创建输出格式上下文
	AVFormatContext* outputFormatContext = nullptr;
	avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, s.URL.toUtf8());
	//选择编码器
	const AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	const AVCodec* audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);


	//初始化视频编解码器上下文

	AVStream* videoInStream = mFPVideo->getVideoInStream();
	AVStream* audioInStream = mFPVideo->getAudioInStream();

	AVCodecContext* videoEncodeContext = avcodec_alloc_context3(videoCodec);
	videoEncodeContext->width = s.outWidth; //1.1
	videoEncodeContext->height = s.outHeight; //1.2
	videoEncodeContext->time_base = videoInStream->time_base;
	videoEncodeContext->framerate = mFPVideo->getVideoCtx()->framerate;
	videoEncodeContext->pix_fmt = videoCodec->pix_fmts[0];
	videoEncodeContext->sample_aspect_ratio = mFPVideo->getVideoCtx()->sample_aspect_ratio;
	videoEncodeContext->has_b_frames = mFPVideo->getVideoCtx()->has_b_frames;
	videoEncodeContext->bit_rate = s.videoBitRate; //1.3
	videoEncodeContext->codec_type = mFPVideo->getVideoCtx()->codec_type;

	// 初始化音频编解码器上下文
	AVCodecContext* audioEncodeContext = avcodec_alloc_context3(audioCodec);
	audioEncodeContext->sample_fmt = audioCodec->sample_fmts[0];
	audioEncodeContext->sample_rate = mFPVideo->getSampleRate();
	audioEncodeContext->channels = mFPVideo->getChannels();
	audioEncodeContext->channel_layout = mFPVideo->getChannelsLayout();
	audioEncodeContext->time_base = {1, mFPVideo->getAudioCtx()->sample_rate};
	audioEncodeContext->bit_rate = s.audioBitRate; //2.1
	audioEncodeContext->codec_type = mFPVideo->getAudioCtx()->codec_type;

	//创建视频流
	AVStream* videoStream = avformat_new_stream(outputFormatContext, videoCodec);
	if (avcodec_parameters_from_context(videoStream->codecpar, videoEncodeContext))return -1;;
	videoStream->codecpar->codec_tag = 0;

	//创建音频流
	AVStream* audioStream = avformat_new_stream(outputFormatContext, audioCodec);
	if (avcodec_parameters_copy(audioStream->codecpar, audioInStream->codecpar))return -1;;
	audioStream->codecpar->codec_tag = 0;


	if (avcodec_open2(videoEncodeContext, videoCodec, nullptr) < 0)return -1;
	if (avcodec_open2(audioEncodeContext, audioCodec, nullptr) < 0)return -1;

	if (avio_open(&outputFormatContext->pb, s.URL.toUtf8(),AVIO_FLAG_WRITE) < 0)
		return -1;
	if (avformat_write_header(outputFormatContext, nullptr) < 0)return -1;
	//创建缩放上下文
	SwsContext* sws = sws_getContext(
		mFPVideo->getVideoCtx()->width,
		mFPVideo->getVideoCtx()->height,
		mFPVideo->getVideoCtx()->pix_fmt,
		videoEncodeContext->width,
		videoEncodeContext->height,
		videoEncodeContext->pix_fmt,
		SWS_FAST_BILINEAR,
		nullptr, nullptr, nullptr
	);
	//写入
	AVFrame* frame = nullptr;
	AVPacket* packet = av_packet_alloc();
	int ret = mFPVideo->getNextInfo(frame, 1);
	qint64 temp = frame
		              ? MFPVideo::toMsec(frame->pts, &(ret == 2 ? videoInStream->time_base : audioInStream->time_base))
		              : 0;
	bool b1 = true, b2 = true;
	qint64 s1 = 0, s2 = 0;
	while (ret > 0 && !isStop && temp < s.endPts) {
		if (ret > 1 && av_frame_make_writable(frame) < 0) { return -1; }

		if (ret == 2 && !s.closeVideo && temp >= s.startPts) {
			if (b1)s1 = frame->pts;
			frame->pts -= s1;
			sws_scale(sws, frame->data, frame->linesize, 0, frame->height, frame->data, frame->linesize);
			writeFrame(packet, frame, videoInStream, videoStream, videoEncodeContext, outputFormatContext);
			b1 = false;
		}
		else if (ret == 3 && !s.closeAudio && temp >= s.startPts) {
			if (b2)s2 = frame->pts;
			frame->pts -= s2;
			writeFrame(packet, frame, audioInStream, audioStream, audioEncodeContext, outputFormatContext);
			b2 = false;
		}
		else if (ret > 1)
			av_frame_free(&frame);
		ret = mFPVideo->getNextInfo(frame, 1);
		temp = frame
			       ? MFPVideo::toMsec(frame->pts, &(ret == 2 ? videoInStream->time_base : audioInStream->time_base))
			       : temp;
		emit progress(temp);
	}
	//防止中途退出造成内存泄漏
	emit progress(s.endPts);
	if (ret > 0)
		av_frame_free(&frame);
	writeFrame(packet, nullptr, videoInStream, videoStream, videoEncodeContext, outputFormatContext);
	writeFrame(packet, nullptr, audioInStream, audioStream, audioEncodeContext, outputFormatContext);
	av_write_trailer(outputFormatContext);

	// 清理资源
	avcodec_free_context(&videoEncodeContext);
	avcodec_free_context(&audioEncodeContext);
	avio_closep(&outputFormatContext->pb);
	avformat_free_context(outputFormatContext);
	av_packet_free(&packet);
	sws_freeContext(sws);
	return 0;
}
