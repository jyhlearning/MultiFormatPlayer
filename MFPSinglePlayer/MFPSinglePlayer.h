#pragma once

#include "mfpsingleplayer_global.h"
#include "MFPluginBase.h"
#include "MFPlayerWidget.h"
#include "MFPlayerDecodeThread.h"
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
	MFPlayerWidget* mFPlayerWidget;
	MFPlayerThread* mFPlayerThread;
	MFPAudioThread* mFPAudioThread;
	MFPlayerDecodeThread* mFPlayerDecodeThread;
	MFPlayerThreadState::statement state,stateBefore;
	MFPFrameQueue* frameQueue;
	MFPAudioQueue* audioQueue;
	MFPVideo* mFPVideo;
	MFPSTDClock* clock;
	void stopThreads();
	void startPlay(MFPlayerThreadState::statement state);
	void stopPlay();
public:
	MFPSinglePlayer();
	~MFPSinglePlayer();
	void show() override;

public slots:
	void onPlay();
	void onStop();
	void action(WidgetStete::statement sig);
	void onStateChange(MFPlayerThreadState::statement state);
	void onProgress(qint64 msec);
	void onSpeedChange(double speed);

private slots:
	void destroyThread();
signals:
	void startDecodeThread();
	void startPlayThread(MFPlayerThreadState::statement sig);
	void flagChange(bool state);
};
