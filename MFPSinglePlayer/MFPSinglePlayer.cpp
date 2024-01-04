#include "MFPSinglePlayer.h"
#include "qthread.h"


MFPSinglePlayer::MFPSinglePlayer() {
	frameQueue = new MFPFrameQueue<AVFrame>();
	mFPlayerWidget = new MFPlayerWidget();

	mFPlayerThread = new MFPlayerThread(frameQueue);
	mFPlayerThread->moveToThread(new QThread(this));
	state = MFPlayerThreadState::PAUSE;

	connect(this,SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPlayerThread,
	        SLOT(onPlay(MFPlayerThreadState::statement)));

	connect(mFPlayerThread,SIGNAL(sendFrame(QImage)), mFPlayerWidget,SLOT(onFrameChange(QImage)));
	connect(mFPlayerThread,SIGNAL(stateChange(MFPlayerThreadState::statement)), this,
	        SLOT(onStateChange(MFPlayerThreadState::statement)));
	connect(mFPlayerThread,SIGNAL(sendProgress(const qint64)), mFPlayerWidget,
	        SLOT(onProgressChange(const qint64)));

	connect(mFPlayerWidget,SIGNAL(stop()), this,SLOT(onStop()));
	connect(mFPlayerWidget,SIGNAL(play(WidgetStete::statement)), this,SLOT(action(WidgetStete::statement)));
	connect(mFPlayerWidget,SIGNAL(destroyed()), this, SLOT(destroyThread()));
	connect(mFPlayerWidget,SIGNAL(progress(qint64)), this, SLOT(onProgress(qint64)));
	connect(mFPlayerWidget,SIGNAL(speed(double)), this, SLOT(onSpeedChange(double)));
	mFPlayerThread->thread()->start();
	mFPlayerWidget->setSliderRange(0, frameQueue->getTotalTime());
	mFPlayerWidget->setBackwardLable(frameQueue->getTotalTime());
}

MFPSinglePlayer::~MFPSinglePlayer() {
	delete mFPlayerWidget;
	delete frameQueue;
}

void MFPSinglePlayer::stopThreads() {
	stopPlay();
	if (mFPlayerThread->thread()->isRunning()) {
		mFPlayerThread->thread()->quit();
		mFPlayerThread->thread()->wait();
	}
}

void MFPSinglePlayer::startPlay(MFPlayerThreadState::statement state) {
	//确保完全退出后再执行启动
	frameQueue->playLock.lock();
	mFPlayerThread->setFlag(false);
	emit startPlayThread(state);
	frameQueue->playLock.unlock();
}

void MFPSinglePlayer::stopPlay() {
	stateBefore = state;
	frameQueue->forceOut();
	mFPlayerThread->setFlag(true);
}

void MFPSinglePlayer::show() { mFPlayerWidget->show(); }

void MFPSinglePlayer::destroyThread() {
	stopThreads();
	delete mFPlayerThread;
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
	if (stateBefore == MFPlayerThreadState::PLAYING)
		startPlay(MFPlayerThreadState::CONTINUEPLAY);
	else {
		frameQueue->setLastPts(msec);
		startPlay(MFPlayerThreadState::NEXTFRAME);
	}
}

void MFPSinglePlayer::onSpeedChange(double speed) {
	stopPlay();
	frameQueue->setSpeed(speed);
	if (stateBefore == MFPlayerThreadState::PLAYING)
		startPlay(MFPlayerThreadState::CONTINUEPLAY);
}
