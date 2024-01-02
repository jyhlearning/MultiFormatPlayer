#include "MFPSinglePlayer.h"
#include "qthread.h"


MFPSinglePlayer::MFPSinglePlayer() {
	frameQueue = new MFPFrameQueue<AVFrame>();
	mFPlayerWidget = new MFPlayerWidget();
	mFPlayerDecodeThread = new MFPlayerDecodeThread(frameQueue);
	mFPlayerDecodeThread->moveToThread(new QThread(this));

	mFPlayerThread = new MFPlayerThread(frameQueue);
	mFPlayerThread->moveToThread(new QThread(this));
	state = PAUSE;

	connect(this,SIGNAL(startPlayThread()), mFPlayerThread,SLOT(onPlay()));
	connect(this,SIGNAL(startDecodeThread()), mFPlayerDecodeThread,SLOT(decode()));
	connect(mFPlayerThread,SIGNAL(sendFrame(QImage)), mFPlayerWidget,SLOT(onFrameChange(QImage)));
	connect(mFPlayerThread,SIGNAL(stateChange(statement)), this,SLOT(onStateChange(statement)));
	connect(mFPlayerThread,SIGNAL(sendProgress(const qint64, const qint64)), mFPlayerWidget,
	        SLOT(onProgressChange(const qint64, const qint64)));
	connect(mFPlayerWidget,SIGNAL(play()), this,SLOT(onPlay()));
	connect(mFPlayerWidget,SIGNAL(destroyed()), this, SLOT(destroyThread()));
	connect(mFPlayerWidget, SIGNAL(progress(int)), this, SLOT(onProgress(int)));
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

void MFPSinglePlayer::startPlay()
{
	mFPlayerThread->setFlag(false);
	mFPlayerDecodeThread->setFlag(false);
	if(frameQueue->playEnd) {
		emit startDecodeThread();
	}
	emit startPlayThread();
}

void MFPSinglePlayer::stopPlay()
{
	mFPlayerThread->setFlag(true);
}

void MFPSinglePlayer::show() { mFPlayerWidget->show(); }

void MFPSinglePlayer::destroyThread() {
	stopThreads();
	delete mFPlayerDecodeThread;
	delete mFPlayerThread;
}

void MFPSinglePlayer::onPlay() {
	switch (state) {
	case PAUSE:
		startPlay();
		break;
	case PLAYING:
		stopPlay();
		break;
	}
}

void MFPSinglePlayer::onStateChange(statement state) {
	this->state = state;
	switch (state) {
	case PAUSE:
		mFPlayerWidget->changeButton("PLAY");
		break;
	case PLAYING:
		mFPlayerWidget->changeButton("PAUSE");
		break;
	}
}

void MFPSinglePlayer::onProgress(int msec) {
	mFPlayerDecodeThread->setFlag(true);
	mFPlayerThread->setFlag(true);
	mFPlayerDecodeThread->onControlProgress(msec);
	//startPlay();
}
