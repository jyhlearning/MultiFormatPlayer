#include "MFPSinglePlayer.h"
#include "qthread.h"
#include "QJsonArray"
#include <QMessageBox>

MFPSinglePlayer::MFPSinglePlayer() {
	capacity = 30;
	frameQueue = new MFPFrameQueue(capacity);
	audioQueue = new MFPAudioQueue(capacity);
	clock = new MFPSTDClock;
	mFPVideo = new MFPVideo;

	mFPlayerWidget = new MFPlayerWidget;
	mFPlayerThread = new MFPlayerThread(frameQueue, clock);
	mFPAudioThread = new MFPAudioThread(audioQueue, clock);
	mFPlayerDecodeThread = new MFPlayerDecodeThread(frameQueue, audioQueue, mFPVideo);
	mFPlayerEncodeThread = new MFPlayerEncoderThread(mFPVideo);

	mFPlayerThread->moveToThread(new QThread(this));

	mFPAudioThread->moveToThread(new QThread(this));

	mFPlayerDecodeThread->moveToThread(new QThread(this));

	mFPlayerEncodeThread->moveToThread(new QThread(this));

	state = MFPlayerThreadState::PAUSE;

	connect(this, SIGNAL(startEncodeThread()), mFPlayerEncodeThread, SLOT(encode()));
	connect(this, SIGNAL(startDecodeThread(int,qint64)), mFPlayerDecodeThread, SLOT(decode(int,qint64)));
	connect(this, SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPAudioThread,
	        SLOT(onPlay(MFPlayerThreadState::statement)));
	connect(this, SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPlayerThread,
	        SLOT(onPlay(MFPlayerThreadState::statement)));
	connect(this, SIGNAL(error(QString, QString)), mFPlayerWidget, SLOT(onError(QString,QString)));
	connect(this, SIGNAL(changeButton(QString)), mFPlayerWidget, SLOT(onChangeButton(QString)));

	connect(mFPlayerThread, SIGNAL(sendFrame(QImage)), mFPlayerWidget,SLOT(onFrameChange(QImage)));
	connect(mFPlayerThread, SIGNAL(stateChange(MFPlayerThreadState::statement)), this,
	        SLOT(onStateChange(MFPlayerThreadState::statement)));
	connect(mFPlayerThread, SIGNAL(sendProgress(const qint64)), mFPlayerWidget,
	        SLOT(onProgressChange(const qint64)));
	connect(mFPlayerThread, SIGNAL(release()), this,SLOT(onVideoRelease()), Qt::DirectConnection);
	connect(mFPAudioThread, SIGNAL(release()), this,SLOT(onAudioRelease()), Qt::DirectConnection);

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
	mFPlayerThread->thread()->start();
	mFPAudioThread->thread()->start();
	mFPlayerEncodeThread->thread()->start();
}

MFPSinglePlayer::~MFPSinglePlayer() {
	delete frameQueue;
	delete audioQueue;
	delete mFPVideo;
	delete clock;
}

void MFPSinglePlayer::stopThreads() {
	stopPlay();
	mFPlayerEncodeThread->setFlag(true);
	if (mFPlayerThread->thread()->isRunning()) {
		mFPlayerThread->thread()->quit();
		mFPlayerThread->thread()->wait();
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

void MFPSinglePlayer::startPlay(MFPlayerThreadState::statement state, int option) {
	if (!mFPVideo->isParse()) {
		emit error("warning!", "You didn't choose a reasonable resource");
		return;
	}
	frameQueue->frameLock.lock();
	audioQueue->frameLock.lock();
	//确保完全退出后再执行启动
	mFPlayerThread->setFlag(false);
	mFPAudioThread->setFlag(false);
	mFPlayerDecodeThread->setFlag(false);
	frameQueue->initQueue();
	audioQueue->initQueue();
	clock->init();
	frameQueue->setPlayEnd(false);
	audioQueue->setPlayEnd(false);
	//查看上一个pts
	mFPVideo->jumpTo(clock->getLastPts(), option);
	if (option == ROUGH) { clock->setLastPts(clock->getStartPts()); }
	emit startDecodeThread(state == MFPlayerThreadState::NEXTFRAME, clock->getLastPts());
	emit startPlayThread(state);
}

void MFPSinglePlayer::stopPlay() {
	//mFPlayerDecodeThread->setFlag(true);
	mFPlayerThread->setFlag(true);
	mFPAudioThread->setFlag(true);
	frameQueue->forceGetOut();
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
	mFPVideo->setHwFlag(hwDecode);
	const int ret = mFPVideo->init(url);
	if (ret >= 0) {
		mFPlayerEncodeThread->init();
		mFPAudioThread->init();
		mFPlayerThread->init();

		frameQueue->setCapacity(capacity);
		frameQueue->setFrameRate(mFPVideo->getFrameRate());

		audioQueue->setCapacity(capacity);
		audioQueue->setFrameRate(mFPVideo->getFrameRate());
		audioQueue->setChannels(mFPVideo->getChannels());
		audioQueue->setSampleRate(mFPVideo->getSampleRate());
		audioQueue->setSampleFmt(mFPVideo->getSampleFmt());

		clock->setTotalTime(mFPVideo->getTotalTime());
		const qint64 a = mFPVideo->getAudioStartTime(), b = mFPVideo->getVideoStartTime();
		clock->setStartPts(a >= b ? a : b);
		clock->setLastPts(clock->getStartPts());


		mFPlayerWidget->setInformationDialog({
			mFPVideo->getResolution(), mFPVideo->getTotalTime(), mFPVideo->getFrameRate(), mFPVideo->getChannels()
		});
		mFPlayerWidget->setExprotDialogDefaultUrl(defaultOutputURL);
		mFPlayerWidget->setExportDialogAudioBitRates(audioBitrates);
		mFPlayerWidget->setExportDialogVideoBitRates(videoBitrates);
		mFPlayerWidget->setExportDialogResolution(resolutions);
		mFPlayerWidget->setExportDialogFormat(formats);
		mFPlayerWidget->setExportVideoSettings(mFPlayerEncodeThread->exportDefaultProfile());
		mFPlayerWidget->setSliderRange(0, clock->getTotalTime());
		mFPlayerWidget->setBackwardLable(clock->getTotalTime());
	}
	if (ret >= 0)
		startPlay(MFPlayerThreadState::NEXTFRAME);
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
	delete mFPlayerThread;
	delete mFPAudioThread;
	delete mFPlayerDecodeThread;
	delete mFPlayerEncodeThread;
}

void MFPSinglePlayer::onPlay() {
	switch (state) {
	case MFPlayerThreadState::PAUSE:
		startPlay(MFPlayerThreadState::CONTINUEPLAY);
		break;
	case MFPlayerThreadState::PLAYING:
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
		if (frameQueue->getPlayEnd() && audioQueue->getPlayEnd()) { clock->setLastPts(clock->getStartPts()); }
		onPlay();
		break;
	case WidgetStete::NEXTFRAME:
		stopPlay();
		clock->setLastPts(clock->getLastPts() + 1000 / frameQueue->getFrameRate());
		startPlay(MFPlayerThreadState::NEXTFRAME);
		break;
	case WidgetStete::LASTFRAME:
		stopPlay();
		clock->setLastPts(clock->getLastPts() - 1000 / frameQueue->getFrameRate());
		startPlay(MFPlayerThreadState::NEXTFRAME);
		break;
	default: ;
	}
}

void MFPSinglePlayer::onStateChange(MFPlayerThreadState::statement state) {
	this->state = state;
	switch (state) {
	case MFPlayerThreadState::PAUSE:
		emit changeButton("PLAY");
		break;
	case MFPlayerThreadState::PLAYING:
		emit changeButton("PAUSE");
		break;
	default: ;
	}
}

void MFPSinglePlayer::onProgress(qint64 msec) {
	clock->setLastPts(msec);
	//如果跳转的时间超过了总时间，就跳转最近的关键帧，因为最后不一定能跳转，导致在视觉上有差异
	if (msec >= clock->getTotalTime())
		startPlay(MFPlayerThreadState::CONTINUEPLAY, ROUGH);
	else if (stateBefore == MFPlayerThreadState::PLAYING)
		startPlay(MFPlayerThreadState::CONTINUEPLAY, PRECISE);
	else startPlay(MFPlayerThreadState::NEXTFRAME, PRECISE);
}

void MFPSinglePlayer::onSpeedChange(double speed) {
	onStop();
	frameQueue->setSpeed(speed);
	audioQueue->setSpeed(speed);
	if (stateBefore == MFPlayerThreadState::PLAYING)
		startPlay(MFPlayerThreadState::CONTINUEPLAY);
}

void MFPSinglePlayer::onExports(settings s) {
	if (!mFPVideo->isParse()) {
		QMessageBox::warning(mFPlayerWidget, tr("警告!"), tr("请选择可用的资源文件"),
		                     QMessageBox::Close);
		return;
	}
	stopPlay();
	frameQueue->frameLock.lock();
	audioQueue->frameLock.lock();
	mFPVideo->jumpTo(s.startPts);
	mFPlayerEncodeThread->setProfile(s);
	mFPlayerEncodeThread->setFlag(false);
	emit startEncodeThread();
	frameQueue->frameLock.unlock();
	audioQueue->frameLock.unlock();
}

void MFPSinglePlayer::onCancel() { mFPlayerEncodeThread->setFlag(true); }

void MFPSinglePlayer::onFullScreen(bool state) {
	emit sendMessage(state ? FULLSCREEN : WINDOW);
}

void MFPSinglePlayer::onVideoRelease() {
	if (!audioQueue->isQuit()) {
		mFPAudioThread->setFlag(true);
		audioQueue->forceGetOut();
	}
	frameQueue->frameLock.tryLock();
	frameQueue->frameLock.unlock();
}

void MFPSinglePlayer::onAudioRelease() {
	if (frameQueue->isQuit()) {
		mFPlayerThread->setFlag(true);
		frameQueue->forceGetOut();
	}
	audioQueue->frameLock.tryLock();
	audioQueue->frameLock.unlock();
}
