#pragma once
#include "QObject"
#include "MFPDataBase.h"
#include "MFPSTDClock.h"

namespace MFPlayState {
	enum statement {
		PLAYING,
		PAUSE,
		NEXTFRAME,
		CONTINUEPLAY,
		LASTFRAME
	};
}

class MFPlayBase : public QObject {
	Q_OBJECT

private:
	bool isStop;
	MFPDataBase<AVFrame*>* queue;
	MFPSTDClock* clock;
	int playNextFrame(AVFrame*& frame);
	void continousPlayBack();
	virtual void io(AVFrame* frame) = 0;

public:
	MFPlayBase(MFPDataBase<AVFrame*>* queue, MFPSTDClock* clock);
	virtual ~MFPlayBase() = default;
	void setFlag(bool flag);
	virtual void init() =0;

public slots:
	void onPlay(MFPlayState::statement sig);
signals:
	void release();
	void stateChange(MFPlayState::statement state);
	void sendProgress(const qint64 sec);
};
