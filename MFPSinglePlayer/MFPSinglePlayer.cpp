#include "MFPSinglePlayer.h"
#include "qthread.h"


MFPSinglePlayer::MFPSinglePlayer() {
	frameQueue = new MFPFrameQueue;
	audioQueue = new MFPAudioQueue;
	clock = new MFPSTDClock;
	mFPVideo = new MFPVideo;
	mFPVideo->init();

	mFPlayerWidget = new MFPlayerWidget;
	mFPlayerThread = new MFPlayerThread(frameQueue, clock);
	mFPAudioThread = new MFPAudioThread(audioQueue, clock);
	mFPlayerDecodeThread = new MFPlayerDecodeThread(frameQueue, audioQueue, mFPVideo);
	mFPlayerEncodeThread = new MFPlayerEncoderThread(mFPVideo);

	resolutions = new QStringList({"320*240", "640*480", "720*480", "1280*720", "1920*1080", "2560*1440"});
	videoBitrates = new QStringList({"20000000", "7552000", "4992000", "3072000", "2496000"}); //kbps
	audioBitrates = new QStringList({"320000", "128000", "64000"}); //kbps


	frameQueue->setFrameRate(mFPVideo->getFrameRate());
	frameQueue->setTotalTime(mFPVideo->getTotalTime());
	audioQueue->setFrameRate(mFPVideo->getFrameRate());
	audioQueue->setChannels(mFPVideo->getChannels());
	audioQueue->setSampleRate(mFPVideo->getSampleRate());
	audioQueue->setSampleFmt(mFPVideo->getSampleFmt());

	mFPlayerWidget->setInformationDialog({
		mFPVideo->getResolution(), mFPVideo->getTotalTime(), mFPVideo->getFrameRate(), mFPVideo->getChannels()
	});
	mFPlayerWidget->setExportDialogAudioBitRates(*audioBitrates);
	mFPlayerWidget->setExportDialogVideoBitRates(*videoBitrates);
	mFPlayerWidget->setExportDialogResolution(*resolutions);
	mFPlayerWidget->setExportDefaultSettings(mFPlayerEncodeThread->exportDefaultProfile());

	mFPlayerThread->moveToThread(new QThread(this));

	mFPAudioThread->moveToThread(new QThread(this));

	mFPlayerDecodeThread->moveToThread(new QThread(this));

	mFPlayerEncodeThread->moveToThread(new QThread(this));

	state = MFPlayerThreadState::PAUSE;

	connect(this, SIGNAL(startEncodeThread()), mFPlayerEncodeThread, SLOT(encode()));
	connect(this, SIGNAL(startDecodeThread()), mFPlayerDecodeThread, SLOT(decode()));
	connect(this, SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPAudioThread,
	        SLOT(onPlay(MFPlayerThreadState::statement)));
	connect(this, SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPlayerThread,
	        SLOT(onPlay(MFPlayerThreadState::statement)));
	connect(mFPlayerThread,SIGNAL(sendFrame(QImage)), mFPlayerWidget,SLOT(onFrameChange(QImage)));
	connect(mFPlayerThread,SIGNAL(stateChange(MFPlayerThreadState::statement)), this,
	        SLOT(onStateChange(MFPlayerThreadState::statement)));
	connect(mFPlayerThread,SIGNAL(sendProgress(const qint64)), mFPlayerWidget,
	        SLOT(onProgressChange(const qint64)));

	connect(mFPlayerWidget,SIGNAL(destroyed()), this, SLOT(destroyThread()));
	connect(mFPlayerWidget,SIGNAL(stop()), this,SLOT(onStop()));
	connect(mFPlayerWidget,SIGNAL(play(WidgetStete::statement)), this,SLOT(action(WidgetStete::statement)));
	connect(mFPlayerWidget,SIGNAL(progress(qint64)), this, SLOT(onProgress(qint64)));
	connect(mFPlayerWidget,SIGNAL(speed(double)), this, SLOT(onSpeedChange(double)));
	connect(mFPlayerWidget,SIGNAL(exports(settings)), this,SLOT(onExports(settings)));
	connect(mFPlayerWidget,SIGNAL(volume(int)), mFPAudioThread, SLOT(onVolume(int)));
	connect(mFPlayerWidget,SIGNAL(cancel()), this, SLOT(onCancel()));

	connect(mFPlayerEncodeThread, SIGNAL(progress(qint64)), mFPlayerWidget, SLOT(onProgress(qint64)));

	mFPlayerDecodeThread->thread()->start();
	mFPlayerThread->thread()->start();
	mFPAudioThread->thread()->start();
	mFPlayerEncodeThread->thread()->start();

	mFPlayerWidget->setSliderRange(0, frameQueue->getTotalTime());
	mFPlayerWidget->setBackwardLable(frameQueue->getTotalTime());
}

MFPSinglePlayer::~MFPSinglePlayer() {
	delete mFPlayerWidget;
	delete frameQueue;
	delete audioQueue;
	delete mFPVideo;
	delete clock;
	delete videoBitrates;
	delete audioBitrates;
	delete resolutions;
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

void MFPSinglePlayer::startPlay(MFPlayerThreadState::statement state) {
	//确保完全退出后再执行启动
	frameQueue->playLock.lock();
	audioQueue->audioLock.lock();
	frameQueue->decodeLock.lock();
	audioQueue->decodeLock.lock();
	mFPlayerThread->setFlag(false);
	mFPAudioThread->setFlag(false);
	mFPlayerDecodeThread->setFlag(false);
	frameQueue->initQueue();
	audioQueue->initQueue();
	//查看上一个pts
	mFPVideo->jumpTo(frameQueue->getLastPts());
	clock->lock.lock();
	emit startDecodeThread();
	emit startPlayThread(state);
	frameQueue->decodeLock.unlock();
	audioQueue->decodeLock.unlock();
	frameQueue->playLock.unlock();
	audioQueue->audioLock.unlock();
}

void MFPSinglePlayer::stopPlay() {
	stateBefore = state;
	//mFPlayerDecodeThread->setFlag(true);
	mFPlayerThread->setFlag(true);
	mFPAudioThread->setFlag(true);
	frameQueue->forceGetOut();
	audioQueue->forceGetOut();
}

void MFPSinglePlayer::show() { mFPlayerWidget->show(); }

void MFPSinglePlayer::destroyThread() {
	stopThreads();
	delete mFPlayerThread;
	delete mFPAudioThread;
	delete mFPlayerDecodeThread;
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

void MFPSinglePlayer::onStop() { stopPlay(); }

void MFPSinglePlayer::action(WidgetStete::statement sig) {
	qint64 lastPts = 0;

	switch (sig) {
	case WidgetStete::PLAY:
		if (frameQueue->getLastPts() == frameQueue->getTotalTime())
			frameQueue->setLastPts(0);
		onPlay();
		break;
	case WidgetStete::NEXTFRAME:
		stopPlay();
		if (frameQueue->getLastPts() == frameQueue->getTotalTime())
			frameQueue->setLastPts(0);
		else
			frameQueue->setLastPts(frameQueue->getLastPts() + 1);
		if (frameQueue->getLastPts() == frameQueue->getTotalTime())
			audioQueue->setLastPts(0);
		else
			audioQueue->setLastPts(audioQueue->getLastPts() + 1);
		startPlay(MFPlayerThreadState::NEXTFRAME);
		break;
	case WidgetStete::LASTFRAME:
		stopPlay();
		lastPts = frameQueue->getLastPts();
		if (lastPts == 0) { lastPts = frameQueue->getTotalTime(); }
		else if (lastPts == frameQueue->getTotalTime())
			lastPts -= 2000 / frameQueue->getFrameRate() + 1;
		else
			lastPts -= 1000 / frameQueue->getFrameRate() + 1;
		frameQueue->setLastPts(lastPts);
		audioQueue->setLastPts(lastPts);
		startPlay(MFPlayerThreadState::NEXTFRAME);
		break;
	default: ;
	}
}

void MFPSinglePlayer::onStateChange(MFPlayerThreadState::statement state) {
	this->state = state;
	switch (state) {
	case MFPlayerThreadState::PAUSE:
		mFPlayerWidget->changeButton("PLAY");
		break;
	case MFPlayerThreadState::PLAYING:
		mFPlayerWidget->changeButton("PAUSE");
		break;
	default: ;
	}
}

void MFPSinglePlayer::onProgress(qint64 msec) {
	frameQueue->setLastPts(msec);
	audioQueue->setLastPts(msec);
	if (stateBefore == MFPlayerThreadState::PLAYING)
		startPlay(MFPlayerThreadState::CONTINUEPLAY);
	else { startPlay(MFPlayerThreadState::NEXTFRAME); }
}

void MFPSinglePlayer::onSpeedChange(double speed) {
	stopPlay();
	frameQueue->setSpeed(speed);
	audioQueue->setSpeed(speed);
	if (stateBefore == MFPlayerThreadState::PLAYING)
		startPlay(MFPlayerThreadState::CONTINUEPLAY);
}

void MFPSinglePlayer::onExports(settings s) {
	stopPlay();
	mFPVideo->jumpTo(s.startPts);
	mFPlayerEncodeThread->setProfile(s);
	mFPlayerEncodeThread->setFlag(false);
	emit startEncodeThread();
}

void MFPSinglePlayer::onCancel() {
	mFPlayerEncodeThread->setFlag(true);
}
