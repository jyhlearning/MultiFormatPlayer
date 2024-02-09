#include "MFPSinglePlayer.h"
#include "qthread.h"
#include "QJsonArray"
#include <QMessageBox>

MFPSinglePlayer::MFPSinglePlayer() {
	capacity = 30;
	videoQueue = new MFPVideoQueue(capacity);
	audioQueue = new MFPAudioQueue(capacity);
	clock = new MFPSTDClock;
	mFPVideo = new MFPVideo;

	mFPlayerWidget = new MFPlayerWidget;
	mFPVideoThread = new MFPVideoThread(videoQueue, clock);
	mFPAudioThread = new MFPAudioThread(audioQueue, clock);
	mFPlayerDecodeThread = new MFPlayerDecodeThread(videoQueue, audioQueue, mFPVideo);
	mFPlayerEncodeThread = new MFPlayerEncoderThread(mFPVideo);

	mFPVideoThread->moveToThread(new QThread(this));

	mFPAudioThread->moveToThread(new QThread(this));

	mFPlayerDecodeThread->moveToThread(new QThread(this));

	mFPlayerEncodeThread->moveToThread(new QThread(this));

	state = MFPlayState::PAUSE;

	connect(this, SIGNAL(startEncodeThread()), mFPlayerEncodeThread, SLOT(encode()));
	connect(this, SIGNAL(startDecodeThread(int,qint64)), mFPlayerDecodeThread, SLOT(decode(int,qint64)));
	connect(this, SIGNAL(startAudioThread(MFPlayState::statement)), mFPAudioThread,
	        SLOT(onPlay(MFPlayState::statement)));
	connect(this, SIGNAL(startVideoThread(MFPlayState::statement)), mFPVideoThread,
	        SLOT(onPlay(MFPlayState::statement)));
	connect(this, SIGNAL(error(QString, QString)), mFPlayerWidget, SLOT(onError(QString,QString)));
	connect(this, SIGNAL(changeButton(QString)), mFPlayerWidget, SLOT(onChangeButton(QString)));
	connect(this, SIGNAL(loadInitPic()), mFPlayerWidget,SLOT(onloadInitPic()));

	connect(mFPVideoThread, SIGNAL(release()), this,SLOT(onVideoRelease()), Qt::DirectConnection);
	connect(mFPAudioThread, SIGNAL(release()), this,SLOT(onAudioRelease()), Qt::DirectConnection);

	connect(mFPVideoThread, SIGNAL(sendFrame(QImage)), mFPlayerWidget,SLOT(onFrameChange(QImage)));

	connect(mFPlayerWidget, SIGNAL(destroyed()), this, SLOT(destroyThread()));
	connect(mFPlayerWidget, SIGNAL(stop()), this,SLOT(onStop()));
	connect(mFPlayerWidget, SIGNAL(play(WidgetStete::statement)), this,SLOT(action(WidgetStete::statement)));
	connect(mFPlayerWidget, SIGNAL(progress(qint64)), this, SLOT(onProgress(qint64)));
	connect(mFPlayerWidget, SIGNAL(speed(double)), this, SLOT(onSpeedChange(double)));
	connect(mFPlayerWidget, SIGNAL(exports(settings)), this,SLOT(onExports(settings)));
	connect(mFPlayerWidget, SIGNAL(volume(int)), mFPAudioThread, SLOT(onVolume(int)));
	connect(mFPlayerWidget, SIGNAL(cancel()), this, SLOT(onCancel()));
	connect(mFPlayerWidget, SIGNAL(fullScreen(bool)), this, SLOT(onFullScreen(bool)));

	connect(mFPlayerEncodeThread, SIGNAL(progress(qint64)), mFPlayerWidget, SLOT(onProgress(qint64)));

	mFPlayerDecodeThread->thread()->start();
	mFPVideoThread->thread()->start();
	mFPAudioThread->thread()->start();
	mFPlayerEncodeThread->thread()->start();
}

MFPSinglePlayer::~MFPSinglePlayer() {
	delete videoQueue;
	delete audioQueue;
	delete mFPVideo;
	delete clock;
}

void MFPSinglePlayer::stopThreads() {
	stopPlay();
	mFPlayerEncodeThread->setFlag(true);
	if (mFPVideoThread->thread()->isRunning()) {
		mFPVideoThread->thread()->quit();
		mFPVideoThread->thread()->wait();
	}
	if (mFPlayerDecodeThread->thread()->isRunning()) {
		mFPlayerDecodeThread->thread()->quit();
		mFPlayerDecodeThread->thread()->wait();
	}
	if (mFPAudioThread->thread()->isRunning()) {
		mFPAudioThread->thread()->quit();
		mFPAudioThread->thread()->wait();
	}
	if (mFPlayerEncodeThread->thread()->isRunning()) {
		mFPlayerEncodeThread->thread()->quit();
		mFPlayerEncodeThread->thread()->wait();
	}
}

void MFPSinglePlayer::startPlay(MFPlayState::statement state, int option) {
	if (!mFPVideo->isParse()) {
		emit error(tr("警告!"), tr("请选择可用的资源文件"));
		return;
	}
	videoQueue->frameLock.lock();
	audioQueue->frameLock.lock();
	//确保完全退出后再执行启动
	mFPVideoThread->setFlag(false);
	mFPAudioThread->setFlag(false);
	mFPlayerDecodeThread->setFlag(false);
	videoQueue->initQueue();
	audioQueue->initQueue();
	videoQueue->setPlayEnd(false);
	audioQueue->setPlayEnd(false);
	clock->init();
	//查看上一个pts
	mFPVideo->jumpTo(clock->getLastPts(), option);
	emit startDecodeThread(state == MFPlayState::NEXTFRAME, clock->getLastPts());
	emit startVideoThread(state);
	emit startAudioThread(state);
}

void MFPSinglePlayer::stopPlay() {
	//mFPlayerDecodeThread->setFlag(true);
	mFPVideoThread->setFlag(true);
	mFPAudioThread->setFlag(true);
	videoQueue->forceGetOut();
	audioQueue->forceGetOut();
}

void MFPSinglePlayer::readArray(const QString& key, const QJsonObject& obj, QStringList& list) const {
	list.clear();
	QJsonArray a = obj.value(key).toArray();
	for (auto str : a)
		list.append(str.toString());
}

void MFPSinglePlayer::show() { mFPlayerWidget->show(); }

void MFPSinglePlayer::init(const QString& url) {
	stopPlay();
	videoQueue->frameLock.lock();
	audioQueue->frameLock.lock();
	videoQueue->initQueue();
	audioQueue->initQueue();
	mFPVideo->setHwFlag(hwDecode);
	const int ret = mFPVideo->init(url);
	if (ret >= 0) {
		mFPlayerEncodeThread->init();
		mFPAudioThread->init();
		mFPVideoThread->init();
		qint64 a = Q_INT64_C(9223372036854775807), b = Q_INT64_C(9223372036854775807);
		int frameRate = 0, channels = 0;
		std::pair<int, int> resolution = {0, 0};

		if (mFPVideo->getVideoCtx()) {
			videoQueue->setCapacity(capacity);
			videoQueue->setFrameRate(mFPVideo->getFrameRate());
			a = mFPVideo->getVideoStartTime();
			frameRate = mFPVideo->getFrameRate();
			resolution = mFPVideo->getResolution();
		}

		if (mFPVideo->getAudioCtx()) {
			audioQueue->setCapacity(capacity);
			audioQueue->setFrameRate(mFPVideo->getFrameRate());
			audioQueue->setChannels(mFPVideo->getChannels());
			audioQueue->setSampleRate(mFPVideo->getSampleRate());
			audioQueue->setSampleFmt(mFPVideo->getSampleFmt());
			b = mFPVideo->getAudioStartTime();
			channels = mFPVideo->getChannels();
		}

		clock->setTotalTime(mFPVideo->getTotalTime());
		clock->setStartPts(a >= b ? b : a);
		clock->setLastPts(clock->getStartPts());

		disconnect(mFPVideoThread, SIGNAL(stateChange(MFPlayState::statement)), this,
		           SLOT(onStateChange(MFPlayState::statement)));
		disconnect(mFPVideoThread, SIGNAL(sendProgress(const qint64)), mFPlayerWidget,
		           SLOT(onProgressChange(const qint64)));

		disconnect(mFPAudioThread, SIGNAL(stateChange(MFPlayState::statement)), this,
		           SLOT(onStateChange(MFPlayState::statement)));
		disconnect(mFPAudioThread, SIGNAL(sendProgress(const qint64)), mFPlayerWidget,
		           SLOT(onProgressChange(const qint64)));

		if (mFPVideo->getVideoCtx()) {
			connect(mFPVideoThread, SIGNAL(stateChange(MFPlayState::statement)), this,
			        SLOT(onStateChange(MFPlayState::statement)));
			connect(mFPVideoThread, SIGNAL(sendProgress(const qint64)), mFPlayerWidget,
			        SLOT(onProgressChange(const qint64)));
		}
		else if (mFPVideo->getAudioCtx()) {
			connect(mFPAudioThread, SIGNAL(stateChange(MFPlayState::statement)), this,
			        SLOT(onStateChange(MFPlayState::statement)));
			connect(mFPAudioThread, SIGNAL(sendProgress(const qint64)), mFPlayerWidget,
			        SLOT(onProgressChange(const qint64)));
		}


		mFPlayerWidget->setInformationDialog({
			resolution, mFPVideo->getTotalTime(), frameRate, channels
		});
		mFPlayerWidget->setExprotDialogDefaultUrl(defaultOutputURL);
		mFPlayerWidget->setExportDialogAudioBitRates(audioBitrates);
		mFPlayerWidget->setExportDialogVideoBitRates(videoBitrates);
		mFPlayerWidget->setExportDialogResolution(resolutions);
		mFPlayerWidget->setExportDialogFormat(formats);
		mFPlayerWidget->setExportVideoSettings(mFPlayerEncodeThread->exportDefaultProfile());
		mFPlayerWidget->setSliderRange(0, clock->getTotalTime());
		mFPlayerWidget->setBackwardLable(clock->getTotalTime());
		mFPlayerWidget->setForwardLable(0);
		mFPlayerWidget->setTimeSlider(0);
	}
	videoQueue->frameLock.unlock();
	audioQueue->frameLock.unlock();
	emit loadInitPic();
}

void MFPSinglePlayer::setParent(QWidget* parent) { mFPlayerWidget->setParent(parent); }

void MFPSinglePlayer::read(QJsonObject& obj) {
	hwDecode = obj.value("hWDecode").toBool();
	capacity = obj.value("preLoadFrames").toInt();
	defaultOutputURL = obj.value("outputUrl").toString();
	readArray("resolutions", obj, resolutions);
	readArray("videoBitrates", obj, videoBitrates);
	readArray("audioBitrates", obj, audioBitrates);
	readArray("formats", obj, formats);
}

QWidget* MFPSinglePlayer::getInstance() { return mFPlayerWidget; }

void MFPSinglePlayer::destroyThread() {
	stopThreads();
	delete mFPVideoThread;
	delete mFPAudioThread;
	delete mFPlayerDecodeThread;
	delete mFPlayerEncodeThread;
}

void MFPSinglePlayer::onPlay() {
	switch (state) {
	case MFPlayState::PAUSE:
		startPlay(MFPlayState::CONTINUEPLAY);
		break;
	case MFPlayState::PLAYING:
		stopPlay();
		break;
	}
}

void MFPSinglePlayer::onStop() {
	stateBefore = state;
	stopPlay();
}

void MFPSinglePlayer::action(WidgetStete::statement sig) {
	if (!mFPVideo->isParse()) {
		emit error(tr("警告!"), tr("请选择可用的资源文件"));
		return;
	}
	switch (sig) {
	case WidgetStete::PLAY:
		if (judge(mFPVideo->getVideoCtx(), mFPVideo->getAudioCtx(), videoQueue->getPlayEnd(),
		          audioQueue->getPlayEnd())) { clock->setLastPts(clock->getStartPts()); }
		onPlay();
		break;
	case WidgetStete::NEXTFRAME:
		stopPlay();
		clock->setLastPts(clock->getLastPts() + 1000 / videoQueue->getFrameRate());
		startPlay(MFPlayState::NEXTFRAME);
		break;
	case WidgetStete::LASTFRAME:
		stopPlay();
		clock->setLastPts(clock->getLastPts() - 1000 / videoQueue->getFrameRate());
		startPlay(MFPlayState::NEXTFRAME);
		break;
	default: ;
	}
}

void MFPSinglePlayer::onStateChange(MFPlayState::statement state) {
	this->state = state;
	switch (state) {
	case MFPlayState::PAUSE:
		emit changeButton("PLAY");
		break;
	case MFPlayState::PLAYING:
		emit changeButton("PAUSE");
		break;
	default: ;
	}
}

void MFPSinglePlayer::onProgress(qint64 msec) {
	clock->setLastPts(msec);
	//如果跳转的时间超过了总时间，就跳转最近的关键帧，因为最后不一定能跳转，导致在视觉上有差异
	if (msec >= clock->getTotalTime())
		startPlay(MFPlayState::CONTINUEPLAY, ROUGH);
	else if (stateBefore == MFPlayState::PLAYING)
		startPlay(MFPlayState::CONTINUEPLAY, ROUGH);
	else startPlay(MFPlayState::NEXTFRAME);
}

void MFPSinglePlayer::onSpeedChange(double speed) {
	onStop();
	videoQueue->setSpeed(speed);
	audioQueue->setSpeed(speed);
	if (stateBefore == MFPlayState::PLAYING)
		startPlay(MFPlayState::CONTINUEPLAY);
}

void MFPSinglePlayer::onExports(settings s) {
	if (!mFPVideo->isParse()) {
		QMessageBox::warning(mFPlayerWidget, tr("警告!"), tr("请选择可用的资源文件"),
		                     QMessageBox::Close);
		return;
	}
	stopPlay();
	videoQueue->frameLock.lock();
	audioQueue->frameLock.lock();
	mFPVideo->jumpTo(s.startPts);
	mFPlayerEncodeThread->setProfile(s);
	mFPlayerEncodeThread->setFlag(false);
	emit startEncodeThread();
	videoQueue->frameLock.unlock();
	audioQueue->frameLock.unlock();
}

void MFPSinglePlayer::onCancel() { mFPlayerEncodeThread->setFlag(true); }

void MFPSinglePlayer::onFullScreen(bool state) {
	emit sendMessage(state ? FULLSCREEN : WINDOW);
}

void MFPSinglePlayer::onVideoRelease() { videoQueue->frameLock.unlock(); }

void MFPSinglePlayer::onAudioRelease() { audioQueue->frameLock.unlock(); }

bool MFPSinglePlayer::judge(const bool a, const bool b, const bool q1, const bool q2) {
	return (a && b && q1 && q2) || ((a && !b) && q1) || ((!a && b) && q2);
}
