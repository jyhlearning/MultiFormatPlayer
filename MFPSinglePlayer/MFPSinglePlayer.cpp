#include "MFPSinglePlayer.h"
#include "qthread.h"


MFPSinglePlayer::MFPSinglePlayer()
{
    frameQueue = new MFPFrameQueue<AVFrame>();
	mFPlayerWidget = new MFPlayerWidget();
    mFPlayerDecodeThread = new MFPlayerDecodeThread(frameQueue);
    mFPlayerDecodeThread->moveToThread(new QThread(this));

    mFPlayerThread = new MFPlayerThread(frameQueue);
    mFPlayerThread->moveToThread(new QThread(this));
    state = PAUSE;

    connect(this,SIGNAL(startThread()),mFPlayerDecodeThread,SLOT(decode()));
    connect(this,SIGNAL(startThread()),mFPlayerThread,SLOT(onPlay()));
    connect(mFPlayerThread,SIGNAL(sendFrame(QImage)), mFPlayerWidget,SLOT(onFrameChange(QImage)));
    connect(mFPlayerThread,SIGNAL(stateChange(statement)),this,SLOT(onStateChange(statement)));
    connect(mFPlayerWidget,SIGNAL(play()),this,SLOT(onPlay()));
    connect(mFPlayerWidget,SIGNAL(destroyed()), this, SLOT(destroyThread()));
}

MFPSinglePlayer::~MFPSinglePlayer() {
    delete mFPlayerWidget;
    delete frameQueue;
}

void MFPSinglePlayer::stopThread(){
    if (mFPlayerDecodeThread->thread()->isRunning()) {
        mFPlayerDecodeThread->setFlag(true);
        mFPlayerDecodeThread->thread()->quit();
        mFPlayerDecodeThread->thread()->wait();
    }
    if (mFPlayerThread->thread()->isRunning()) {
        mFPlayerThread->setFlag(true);
        mFPlayerThread->thread()->quit();
        mFPlayerThread->thread()->wait();
    }
}
void MFPSinglePlayer::show() {
	mFPlayerWidget->show();
}

void MFPSinglePlayer::destroyThread() {
    stopThread();
    delete mFPlayerDecodeThread;
    delete mFPlayerThread;
}

void MFPSinglePlayer::onPlay() {
    switch (state) {
    case PAUSE:
        mFPlayerDecodeThread->setFlag(false);
        mFPlayerThread->setFlag(false);
    	mFPlayerDecodeThread->thread()->start();
        mFPlayerThread->thread()->start();

        state = PLAYING;
        emit startThread();
        break;
    case PLAYING:
        stopThread();
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

