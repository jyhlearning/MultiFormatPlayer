#pragma once
#include "MFPVideo.h"
#include "QObject"
#include "MFPExportSettings.h"
class MFPlayerEncoderThread:public QObject
{
	Q_OBJECT
private:
	MFPVideo* mFPVideo;
	bool isStop;
	settings s;
	void init();
	int writeFrame(AVPacket* packet,AVFrame* frame, AVStream* inStream,AVStream* outStream,AVCodecContext* context,AVFormatContext* fmtCtx);

public:
	void setFlag(bool flag);
	void setProfile(const settings& s);
	settings exportDefaultProfile();
	MFPlayerEncoderThread(MFPVideo* mFPVideo);
public slots:
	int encode();
signals:
	void progress(qint64 p);

};

