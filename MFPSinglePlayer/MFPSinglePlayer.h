#pragma once

#include "mfpsingleplayer_global.h"
#include "MFPluginBase.h"
#include "MFPlayerWidget.h"
#include "MFPlayerDecodeThread.h"
#include "MFPlayerEncoderThread.h"
#include "MFPVideoQueue.h"
#include "MFPAudioQueue.h"
#include "MFPAudioThread.h"
#include "MFPVideoThread.h"
#include "MFPlayBase.h"
//#include "opencv2/highgui/highgui.hpp"

class MFPSINGLEPLAYER_EXPORT MFPSinglePlayer :  public MFPluginBase {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID MFPluginBase_IID)
	Q_INTERFACES(MFPluginBase)

private:
	MFPlayerDecodeThread* mFPlayerDecodeThread;
	MFPlayerEncoderThread* mFPlayerEncodeThread;
	MFPlayBase* mFPAudioThread;
	MFPlayBase* mFPVideoThread;
	MFPlayerWidget* mFPlayerWidget;
	MFPVideoQueue* videoQueue;
	MFPAudioQueue* audioQueue;
	MFPVideo* mFPVideo;
	MFPSTDClock* clock;
	QStringList resolutions;
	QStringList audioBitrates;
	QStringList videoBitrates;
	QStringList formats;
	QString defaultOutputURL;
	int capacity;
	bool hwDecode;
	MFPlayState::statement state,stateBefore;
	void stopPlay();
	void stopThreads();
	void startPlay(MFPlayState::statement state,int option=PRECISE);
	void readArray(const QString& key, const QJsonObject& obj, QStringList& list)const;
	static bool judge(const bool a,const bool b,const bool q1,const bool q2);
public:
	MFPSinglePlayer();
	~MFPSinglePlayer();
	void show() override;
	void init(const QString& url) override;
	void setParent(QWidget* parent) override;
	void read(QJsonObject& obj) override;
	QWidget* getInstance() override;
public slots:
	void onPlay();
	void onStop();
	void action(WidgetStete::statement sig);
	void onStateChange(MFPlayState::statement state);
	void onProgress(qint64 msec);
	void onSpeedChange(double speed);
	void onExports(settings s);
	void onCancel();
	void onFullScreen(bool state);
	void onVideoRelease();
	void onAudioRelease();

private slots:
	void destroyThread();
signals:
	void startDecodeThread(const int option, const qint64 lPts);
	void startAudioThread(MFPlayState::statement sig);
	void startVideoThread(MFPlayState::statement sig);
	void startEncodeThread();
	void flagChange(bool state);
	void error(const QString title,const QString info);
	void changeButton(QString s);
	void sendMessage(option o) override;
	void loadInitPic();
};
