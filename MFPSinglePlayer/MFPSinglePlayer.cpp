#include "MFPSinglePlayer.h"
#include "qthread.h"


MFPSinglePlayer::MFPSinglePlayer() {
	frameQueue = new MFPFrameQueue<AVFrame>();
	mFPlayerWidget = new MFPlayerWidget();
	mFPlayerDecodeThread = new MFPlayerDecodeThread(frameQueue);
	mFPlayerDecodeThread->moveToThread(new QThread(this));

	mFPlayerThread = new MFPlayerThread(frameQueue);
	mFPlayerThread->moveToThread(new QThread(this));
	state = MFPlayerThreadState::PAUSE;

	connect(this,SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPlayerThread,SLOT(onPlay(MFPlayerThreadState::statement)));
	connect(this,SIGNAL(startDecodeThread()), mFPlayerDecodeThread,SLOT(decode()));

	connect(mFPlayerThread,SIGNAL(sendFrame(QImage)), mFPlayerWidget,SLOT(onFrameChange(QImage)));
	connect(mFPlayerThread,SIGNAL(stateChange(MFPlayerThreadState::statement)), this,SLOT(onStateChange(MFPlayerThreadState::statement)));
	connect(mFPlayerThread,SIGNAL(sendProgress(const qint64, const qint64)), mFPlayerWidget,
	        SLOT(onProgressChange(const qint64, const qint64)));

	connect(mFPlayerWidget,SIGNAL(play(WidgetStete::statement)), this,SLOT(action(WidgetStete::statement)));
	connect(mFPlayerWidget,SIGNAL(destroyed()), this, SLOT(destroyThread()));
	connect(mFPlayerWidget, SIGNAL(progress(qint64)), this, SLOT(onProgress(qint64)));
	startThreads();
}

MFPSinglePlayer::~MFPSinglePlayer() {
	delete mFPlayerWidget;
	delete frameQueue;
}

void MFPSinglePlayer::stopThreads() {
	mFPlayerDecodeThread->setFlag(true);
	mFPlayerThread->setFlag(true);
	frameQueue->forceOut();
	if (mFPlayerDecodeThread->thread()->isRunning()) {
		mFPlayerDecodeThread->thread()->quit();
		mFPlayerDecodeThread->thread()->wait();
	}
	if (mFPlayerThread->thread()->isRunning()) {
		mFPlayerThread->thread()->quit();
		mFPlayerThread->thread()->wait();
	}
}

void MFPSinglePlayer::startThreads() {
	mFPlayerDecodeThread->thread()->start();
	mFPlayerThread->thread()->start();
}

void MFPSinglePlayer::startPlay(MFPlayerThreadState::statement state) {
	//确保完全退出后再执行启动
	frameQueue->playLock.lock();
	mFPlayerThread->setFlag(false);
	mFPlayerDecodeThread->setFlag(false);
	if (frameQueue->playEnd) { emit startDecodeThread(); }
	emit startPlayThread(state);
	frameQueue->playLock.unlock();
}

void MFPSinglePlayer::stopPlay() { mFPlayerThread->setFlag(true); }

void MFPSinglePlayer::show() { mFPlayerWidget->show(); }

void MFPSinglePlayer::destroyThread() {
	stopThreads();
	delete mFPlayerDecodeThread;
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

void MFPSinglePlayer::action(WidgetStete::statement sig) {
	qint64 lastPts = 0;


	switch (sig) {
	case WidgetStete::PLAY:
		if (frameQueue->getLastPts() == frameQueue->getTotalTime())
			frameQueue->setLastPts(0);
		onPlay();
		break;
	case WidgetStete::NEXTFRAME:
		if (frameQueue->getLastPts() == frameQueue->getTotalTime())
			frameQueue->setLastPts(0);
		stopPlay();
		startPlay(MFPlayerThreadState::NEXFRAME);
		break;
	case WidgetStete::LASTFRAME:
		lastPts = frameQueue->getLastPts();
		if(lastPts == 0) {
			lastPts = frameQueue->getTotalTime();
		}else if(lastPts==frameQueue->getTotalTime())
			lastPts -= 2000 / frameQueue->getFrameRate() + 1;
		else
			lastPts -= 1000 / frameQueue->getFrameRate() + 1 ;
		onProgress(lastPts);
		startPlay(MFPlayerThreadState::NEXFRAME);
		break;
	default:
		break;
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
	}
}

void MFPSinglePlayer::onProgress(qint64 msec) {
	mFPlayerDecodeThread->setFlag(true);
	mFPlayerThread->setFlag(true);
	mFPlayerDecodeThread->onControlProgress(msec);
	//startPlay();
}
