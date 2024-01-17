#include "MFPVideo.h"

int MFPVideo::init(const QString& url) {
	freeResources();

	const std::string str = url.toStdString();

	videoPath = str.c_str();
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

	pAVctx->thread_count = 4;
	pAVctx->flags2 |= AV_CODEC_FLAG2_FAST;
	aAVctx->thread_count = 4;
	aAVctx->flags2 |= AV_CODEC_FLAG2_FAST;


	//查找解码器
	avcodec_parameters_to_context(pAVctx, pFormatCtx->streams[videoIndex]->codecpar);
	avcodec_parameters_to_context(aAVctx, pFormatCtx->streams[audioIndex]->codecpar);

	pCodec = avcodec_find_decoder(pAVctx->codec_id);
	aCodec = avcodec_find_decoder(aAVctx->codec_id);
	if (hwFlag)
		initHWDecoder(pCodec, pAVctx);
	if (pCodec == nullptr || aCodec == nullptr) {
		avcodec_close(pAVctx);
		avformat_close_input(&pFormatCtx);
		qDebug("avcodec_find_decoder err.");
		return -4;
	}

	//初始化pAVctx,aAVctx
	if (avcodec_open2(pAVctx, pCodec, nullptr) < 0 || avcodec_open2(aAVctx, aCodec, nullptr) < 0) {
		avcodec_close(pAVctx);
		avformat_close_input(&pFormatCtx);
		qDebug("avcodec_open2 err.");
		return -5;
	}

	//初始化pAVpkt
	pAVpkt = av_packet_alloc();

	//初始化数据帧空间
	pAVframe = av_frame_alloc();

	//暂时用不到
	//avFrameToOpenCVBGRSwsContext = sws_getContext(
	//	pAVctx->width,
	//	pAVctx->height,
	//	pAVctx->pix_fmt,
	//	pAVctx->width,
	//	pAVctx->height,
	//	AVPixelFormat::AV_PIX_FMT_BGR24,
	//	SWS_FAST_BILINEAR,
	//	nullptr, nullptr, nullptr
	//);

	totalTime = 1000 * pFormatCtx->duration / AV_TIME_BASE;
	parse = true;
	return 0;
}

MFPVideo::MFPVideo() {
	hwFlag = false;
	videoIndex = -1;
	audioIndex = -1;
	parse = false;
	pAVctx = nullptr;
	aAVctx = nullptr;
	pAVframe = nullptr;
	pFormatCtx = nullptr;
	hwBufferRef = nullptr;
}

MFPVideo::~MFPVideo() { freeResources(); }

void MFPVideo::freeResources() {
	//sws_freeContext(avFrameToOpenCVBGRSwsContext);
	if (pAVframe)
		av_frame_free(&pAVframe);
	if (pAVctx)
		avcodec_close(pAVctx);
	if (aAVctx)
		avcodec_close(aAVctx);
	if (pFormatCtx)
		avformat_close_input(&pFormatCtx);
	if (hwBufferRef)
		av_buffer_unref(&hwBufferRef);
	clearBuffer();
	parse = false;
}

void MFPVideo::setHwFlag(bool flag) { hwFlag = flag; }

void MFPVideo::setVideoPath(const QString& path) {
	const std::string str = path.toStdString();
	videoPath = str.c_str();
}

AVPixelFormat get_hw_format(AVCodecContext* s, const enum AVPixelFormat* fmt) {
	Q_UNUSED(s)
	const enum AVPixelFormat* p;

	for (p = fmt; *p != -1; p++) { if (*p == g_pixelFormat) { return *p; } }

	// 当同时打开太多路视频时，如果超过了GPU的能力，可能会返回找不到解码帧格式
	return AV_PIX_FMT_NONE;
}

void MFPVideo::initHWDecoder(const AVCodec* codec, AVCodecContext* ctx) {
	if (!codec) return;

	for (int i = 0; ; i++) {
		const AVCodecHWConfig* config = avcodec_get_hw_config(codec, i); // 检索编解码器支持的硬件配置。

		AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE; // ffmpeg支持的硬件解码器
		QStringList strTypes;
		while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) // 遍历支持的设备类型。
		{
			hwDeviceTypes.append(type);
			const char* ctype = av_hwdevice_get_type_name(type); // 获取AVHWDeviceType的字符串名称。
			if (ctype) { strTypes.append(QString(ctype)); }
		}
		if (!config) {
			return; // 没有找到支持的硬件配置
		}

		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) // 判断是否是设备类型
		{
			for (auto i : hwDeviceTypes) {
				if (config->device_type == AVHWDeviceType(i)) // 判断设备类型是否是支持的硬件解码器
				{
					g_pixelFormat = config->pix_fmt;

					// 打开指定类型的设备，并为其创建AVHWDeviceContext。
					int ret = av_hwdevice_ctx_create(&hwBufferRef, config->device_type, nullptr, nullptr, 0);
					if (ret < 0) { return; }
					ctx->hw_device_ctx = av_buffer_ref(hwBufferRef); // 创建一个对AVBuffer的新引用。
					ctx->get_format = get_hw_format; // 由一些解码器调用，以选择将用于输出帧的像素格式
					return;
				}
			}
		}
	}
}

void MFPVideo::clearBuffer() {
	while(!pQueue.isEmpty()) {
		av_frame_free(&pQueue.front());
		pQueue.pop_front();
	}
	while (!aQueue.isEmpty()) {
		av_frame_free(&aQueue.front());
		aQueue.pop_front();
	}
}

int MFPVideo::getChannels() const { return aAVctx->channels; }

int MFPVideo::getSampleRate() const { return aAVctx->sample_rate; }

AVSampleFormat MFPVideo::getSampleFmt() const { return aAVctx->sample_fmt; }

qint64 MFPVideo::getChannelsLayout() const { return aAVctx->channel_layout; }

qint64 MFPVideo::getVideoStartTime() const { return toMsec(pFormatCtx->streams[videoIndex]->start_time, &pFormatCtx->streams[videoIndex]->time_base); }

qint64 MFPVideo::getAudioStartTime() const { return toMsec(pFormatCtx->streams[audioIndex]->start_time, &pFormatCtx->streams[audioIndex]->time_base);}

AVCodecContext* MFPVideo::getAudioCtx() const { return aAVctx; }

AVCodecContext* MFPVideo::getVideoCtx() const { return pAVctx; }

AVStream* MFPVideo::getVideoInStream() const { return pFormatCtx->streams[videoIndex]; }

AVStream* MFPVideo::getAudioInStream() const { return pFormatCtx->streams[audioIndex]; }

std::pair<int, int> MFPVideo::getResolution() const { return {pAVctx->width, pAVctx->height}; }

qreal MFPVideo::rationalToDouble(const AVRational* rational) {
	const qreal rate = (rational->den == 0) ? 0 : (qreal(rational->num) / rational->den);
	return rate;
}

int MFPVideo::getFrameRate() const {
	return pFormatCtx->streams[videoIndex]->avg_frame_rate.num / pFormatCtx->streams[videoIndex]->avg_frame_rate.
		den;
}

qint64 MFPVideo::getTotalTime() const { return totalTime; }

QByteArray MFPVideo::toQByteArray(const AVFrame* frame, SwrContext* swr_ctx) {
	uint8_t* buffer;
	av_samples_alloc(&buffer, nullptr, av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO),
	                 frame->nb_samples, AV_SAMPLE_FMT_S16, 0);
	swr_convert(swr_ctx, &buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
	QByteArray temp = QByteArray(reinterpret_cast<const char*>(buffer),
	                             av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * av_get_channel_layout_nb_channels(
		                             AV_CH_LAYOUT_STEREO) * frame->nb_samples);
	av_freep(&buffer);
	return temp;
}

bool MFPVideo::isParse() const { return parse; }

int MFPVideo::getNextInfo(AVFrame* & frame, int option) {
	bool flag1 = true, flag2 = true;
	int ret = 0;
	if (av_read_frame(pFormatCtx, pAVpkt) >= 0) {
		//读取一帧未解码的数据
		flag1 = false;
		//如果是视频数据
		if (pAVpkt->stream_index == videoIndex) {
			//解码一帧视频数据
			ret = readFrame(videoIndex, option, pAVctx, pQueue);
		}
		else if (pAVpkt->stream_index == audioIndex) {
			//解码一帧音频数据
			ret = readFrame(audioIndex, option, aAVctx, aQueue);
		}
		av_packet_unref(pAVpkt);
		if (ret < 0)
			return -1;
	}
	else {
		//处理最后buffer中的最后几帧
		pAVpkt->data = nullptr;
		pAVpkt->size = 0;
		readFrame(videoIndex, option, pAVctx, pQueue);
		av_packet_unref(pAVpkt);
	}
	if (!pQueue.isEmpty()) {
		frame = pQueue.front();
		pQueue.pop_front();
		return 2; //正常帧
	}
	else if (!aQueue.isEmpty()) {
		frame = aQueue.front();
		aQueue.pop_front();
		return 3; //正常帧
	}
	else if (flag1 && flag2) {
		return 0; //放完了
	}
	return 1; //空帧
}

int MFPVideo::jumpTo(qint64 msec, int option) {
	// 使用 avformat_seek_file 定位整个文件
	const qint64 min_timestamp = pFormatCtx->start_time != AV_NOPTS_VALUE ? pFormatCtx->start_time : 0;
	const qint64 max_timestamp = INT64_MAX;
	//往前推1s,避免找到的关键帧太大
	if (option == PRECISE)
		msec = msec - 1000 < getVideoStartTime() ? getVideoStartTime() : msec - 1000;
	int ret = avformat_seek_file(pFormatCtx, videoIndex, min_timestamp, av_rescale_q(msec, av_make_q(1, 1000),
		                             pFormatCtx->streams[videoIndex]->time_base), max_timestamp,
	                             AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(pAVctx);

	return ret;
}

int MFPVideo::readFrame(int index, int option, AVCodecContext* ctx, QQueue<AVFrame*>& queue) const {
	if (option == 0) {
		pAVpkt->pts = toMsec(pAVpkt->pts, &pFormatCtx->streams[index]->time_base);
		pAVpkt->dts = toMsec(pAVpkt->dts, &pFormatCtx->streams[index]->time_base);
	}
	if (avcodec_send_packet(ctx, pAVpkt) == 0) {
		// 一个avPacket可能包含多帧数据，所以需要使用while循环一直读取
		while (avcodec_receive_frame(ctx, pAVframe) == 0) {
			AVFrame* dst = av_frame_alloc();
			if (pAVframe->data[0])
				av_frame_move_ref(dst, pAVframe);
			else {
				av_hwframe_map(dst, pAVframe, 0);
				av_hwframe_transfer_data(dst, pAVframe, 0);
				av_frame_copy_props(dst, pAVframe);
				dst->width = pAVframe->width;
				dst->height = pAVframe->height;
				av_frame_unref(pAVframe);
			}
			queue.push_back(dst);
		}
	}
	else {
		qDebug("Decode Error.\n");
		return -1;
	}
	return 0;
}

QImage MFPVideo::toQImage(const AVFrame* frame, SwsContext* avFrameToQImageSwsContext) {
	uchar* buffer = (unsigned char*)
		av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGBA, frame->width, frame->height, 1));
	uchar* data[] = {buffer};
	int lines[4];
	av_image_fill_linesizes(lines, AV_PIX_FMT_RGBA, frame->width); // 使用像素格式pix_fmt和宽度填充图像的平面线条大小。
	sws_scale(avFrameToQImageSwsContext, // 缩放上下文
	          frame->data, // 原图像数组
	          frame->linesize, // 包含源图像每个平面步幅的数组
	          0, // 开始位置
	          frame->height, // 行数
	          data, // 目标图像数组
	          lines); // 包含目标图像每个平面的步幅的数组
	const QImage re = QImage(buffer, frame->width, frame->height, QImage::Format_RGBA8888).copy();
	av_free(buffer);
	return re;
}

cv::Mat MFPVideo::AVFrameToMat(const AVFrame* frame, SwsContext* avFrameToOpenCVBGRSwsContext) {
	const int image_width = frame->width;
	const int image_height = frame->height;

	cv::Mat resMat(image_height, image_width, CV_8UC3);
	int cvLinesizes[1];
	cvLinesizes[0] = resMat.step1();

	sws_scale(avFrameToOpenCVBGRSwsContext,
	          frame->data,
	          frame->linesize,
	          0,
	          image_height,
	          &resMat.data,
	          cvLinesizes);

	return resMat;
}

qint64 MFPVideo::toMsec(const qint64 msec, const AVRational* rational) {
	return qRound64(
		msec * (1000 * MFPVideo::rationalToDouble(rational)));
}
