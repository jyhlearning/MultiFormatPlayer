#pragma once

#include "mfpsingleplayer_global.h"
#include "MFPluginBase.h"
#include "MFPlayerWidget.h"
#include "MFPlayerDecodeThread.h"
#include "MFPlayerEncoderThread.h"
#include "MFPFrameQueue.h"
#include "MFPAudioQueue.h"
#include "MFPAudioThread.h"
#include "MFPlayerThread.h"
#include "opencv2/highgui/highgui.hpp"

class MFPSINGLEPLAYER_EXPORT MFPSinglePlayer : public QObject, public MFPluginBase {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID MFPluginBase_IID)
	Q_INTERFACES(MFPluginBase)

private:
	MFPlayerDecodeThread* mFPlayerDecodeThread;
	MFPlayerEncoderThread* mFPlayerEncodeThread;
	MFPAudioThread* mFPAudioThread;
	MFPlayerThread* mFPlayerThread;
	MFPlayerWidget* mFPlayerWidget;
	MFPFrameQueue* frameQueue;
	MFPAudioQueue* audioQueue;
	MFPVideo* mFPVideo;
	MFPSTDClock* clock;
	QStringList* resolutions;
	QStringList* audioBitrates;
	QStringList* videoBitrates;
	MFPlayerThreadState::statement state,stateBefore;
	void stopThreads();
	void startPlay(MFPlayerThreadState::statement state);
	void stopPlay();
public:
	MFPSinglePlayer();
	~MFPSinglePlayer();
	void show() override;
	void setParent(QWidget* parent) override;
	QWidget* getParent() override;
public slots:
	void onPlay();
	void onStop();
	void action(WidgetStete::statement sig);
	void onStateChange(MFPlayerThreadState::statement state);
	void onProgress(qint64 msec);
	void onSpeedChange(double speed);
	void onExports(settings s);
	void onCancel();

private slots:
	void destroyThread();
signals:
	void startDecodeThread();
	void startPlayThread(MFPlayerThreadState::statement sig);
	void startEncodeThread();
	void flagChange(bool state);
};
