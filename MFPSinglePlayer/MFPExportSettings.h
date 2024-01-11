#pragma once
struct settings {
	bool closeVideo, closeAudio;
	int outWidth, outHeight;
	qint64 videoBitRate, audioBitRate;
	qint64 startPts, endPts;
	QString URL;
};