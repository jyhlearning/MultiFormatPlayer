#pragma once

#include <QWidget>
#include "ui_MFPlayerWidget.h"

class MFPlayerWidget : public QWidget
{
	Q_OBJECT

public:
	MFPlayerWidget(QWidget *parent = nullptr);
	~MFPlayerWidget();
	void changeButton(QString qString);
signals:
	void play();
	void progress(int msec);
private:
	Ui::MFPlayerWidgetClass ui;
private slots:
	void onPlayButton();
	void onSliderMoved(int v);
public slots:
	void onFrameChange(QImage qImage);
	void onProgressChange(const qint64 sec,const qint64 totalTime);
};
