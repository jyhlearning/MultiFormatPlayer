#pragma once

#include <QWidget>
#include "ui_MFPlayerWidget.h"

namespace WidgetStete {
	enum statement {
		PLAY,
		NEXTFRAME,
		LASTFRAME
	};
}

class MFPlayerWidget : public QWidget {
	Q_OBJECT

public:
	MFPlayerWidget(QWidget* parent = nullptr);
	~MFPlayerWidget();
	void changeButton(QString qString);
signals:
	void play(WidgetStete::statement sig);
	void progress(qint64 msec);

private:
	Ui::MFPlayerWidgetClass ui;

private slots:
	void onPlayButton();
	void onNextFrameButton();
	void onLastFrameButton();
	void onSliderMoved(int v);

public slots:
	void onFrameChange(QImage qImage);
	void onProgressChange(const qint64 sec, const qint64 totalTime);
};
