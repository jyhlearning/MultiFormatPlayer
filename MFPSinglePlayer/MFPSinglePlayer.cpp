﻿#include "MFPSinglePlayer.h"
#include "qthread.h"
#include "QJsonArray"
#include <QMessageBox>


MFPSinglePlayer::MFPSinglePlayer() {
	frameQueue = new MFPFrameQueue;
	audioQueue = new MFPAudioQueue;
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
	connect(this, SIGNAL(startDecodeThread()), mFPlayerDecodeThread, SLOT(decode()));
	connect(this, SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPAudioThread,
	        SLOT(onPlay(MFPlayerThreadState::statement)));
	connect(this, SIGNAL(startPlayThread(MFPlayerThreadState::statement)), mFPlayerThread,
	        SLOT(onPlay(MFPlayerThreadState::statement)));
	connect(this, SIGNAL(error(QString, QString)), mFPlayerWidget, SLOT(onError(QString,QString)));

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
	frameQueue->setPlayEnd(false);
	audioQueue->setPlayEnd(false);
	//查看上一个pts
	mFPVideo->jumpTo(frameQueue->getLastPts(), option);
	if (option == ROUGH) {
		frameQueue->setLastPts(frameQueue->getStartTime());
		audioQueue->setLastPts(audioQueue->getStartTime());
	}
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

void MFPSinglePlayer::readArray(const QString& key, const QJsonObject& obj, QStringList& list) const {
	list.clear();
	QJsonArray a = obj.value(key).toArray();
	for (auto str : a)
		list.append(str.toString());
}

void MFPSinglePlayer::show() { mFPlayerWidget->show(); }

void MFPSinglePlayer::init(const QString& url) {
	stopPlay();
	frameQueue->playLock.lock();
	audioQueue->audioLock.lock();
	const int ret = mFPVideo->init(url);
	if (ret >= 0) {
		mFPlayerEncodeThread->init();

		frameQueue->setFrameRate(mFPVideo->getFrameRate());
		frameQueue->setTotalTime(mFPVideo->getTotalTime());
		frameQueue->setStartTime(mFPVideo->getVideoStartTime());
		audioQueue->setStartTime(mFPVideo->getAudioStartTime());
		audioQueue->setTotalTime(mFPVideo->getTotalTime());
		audioQueue->setFrameRate(mFPVideo->getFrameRate());
		audioQueue->setChannels(mFPVideo->getChannels());
		audioQueue->setSampleRate(mFPVideo->getSampleRate());
		audioQueue->setSampleFmt(mFPVideo->getSampleFmt());

		frameQueue->setLastPts(frameQueue->getStartTime());
		audioQueue->setLastPts(audioQueue->getStartTime());
		mFPlayerWidget->setInformationDialog({
			mFPVideo->getResolution(), mFPVideo->getTotalTime(), mFPVideo->getFrameRate(), mFPVideo->getChannels()
		});
		mFPlayerWidget->setExportDialogAudioBitRates(audioBitrates);
		mFPlayerWidget->setExportDialogVideoBitRates(videoBitrates);
		mFPlayerWidget->setExportDialogResolution(resolutions);
		mFPlayerWidget->setExportDialogFormat(formats);
		mFPlayerWidget->setExportDefaultSettings(mFPlayerEncodeThread->exportDefaultProfile());
		mFPlayerWidget->setSliderRange(0, frameQueue->getTotalTime());
		mFPlayerWidget->setBackwardLable(frameQueue->getTotalTime());
	}

	frameQueue->playLock.unlock();
	audioQueue->audioLock.unlock();
	if (ret >= 0)
		startPlay(MFPlayerThreadState::NEXTFRAME);
}

void MFPSinglePlayer::setParent(QWidget* parent) { mFPlayerWidget->setParent(parent); }

void MFPSinglePlayer::read(QJsonObject& obj) {
	mFPVideo->setHwFlag(obj.value("hWDecode").toBool());
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

void MFPSinglePlayer::onStop() { stopPlay(); }

void MFPSinglePlayer::action(WidgetStete::statement sig) {
	qint64 lastPts = 0;

	switch (sig) {
	case WidgetStete::PLAY:
		if (frameQueue->getPlayEnd() && audioQueue->getPlayEnd()) {
			frameQueue->setLastPts(frameQueue->getStartTime());
			audioQueue->setLastPts(frameQueue->getStartTime());
		}
		onPlay();
		break;
	case WidgetStete::NEXTFRAME:
		stopPlay();
		if (frameQueue->getPlayEnd() && audioQueue->getPlayEnd()) {
			frameQueue->setLastPts(frameQueue->getStartTime());
			audioQueue->setLastPts(frameQueue->getStartTime());
		}
		else {
			frameQueue->setLastPts(frameQueue->getLastPts() + 1);
			audioQueue->setLastPts(audioQueue->getLastPts() + 1);
		}
		startPlay(MFPlayerThreadState::NEXTFRAME);
		break;
	case WidgetStete::LASTFRAME:
		stopPlay();
		if (frameQueue->getLastPts() == frameQueue->getStartTime() || (frameQueue->getPlayEnd() && audioQueue->
			getPlayEnd())) {
			frameQueue->setLastPts(frameQueue->getTotalTime());
			audioQueue->setLastPts(audioQueue->getTotalTime());
			startPlay(MFPlayerThreadState::NEXTFRAME,ROUGH);
		}
		else {
			frameQueue->setLastPts(frameQueue->getLastPts() - 1000 / frameQueue->getFrameRate() - 1);
			audioQueue->setLastPts(audioQueue->getLastPts() - 1000 / audioQueue->getFrameRate() - 1);
			startPlay(MFPlayerThreadState::NEXTFRAME);
		}
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
		startPlay(MFPlayerThreadState::CONTINUEPLAY,ROUGH);
	else { startPlay(MFPlayerThreadState::NEXTFRAME,ROUGH); }
}

void MFPSinglePlayer::onSpeedChange(double speed) {
	stopPlay();
	frameQueue->setSpeed(speed);
	audioQueue->setSpeed(speed);
	if (stateBefore == MFPlayerThreadState::PLAYING)
		startPlay(MFPlayerThreadState::CONTINUEPLAY);
}

void MFPSinglePlayer::onExports(settings s) {
	if (!mFPVideo->isParse()) {
		QMessageBox::warning(mFPlayerWidget, tr("warning!"), tr("You didn't choose a reasonable resource"),
		                     QMessageBox::Close);
		return;
	}
	stopPlay();
	mFPVideo->jumpTo(s.startPts);
	mFPlayerEncodeThread->setProfile(s);
	mFPlayerEncodeThread->setFlag(false);
	emit startEncodeThread();
}

void MFPSinglePlayer::onCancel() { mFPlayerEncodeThread->setFlag(true); }
