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
private:
	Ui::MFPlayerWidgetClass ui;
private slots:
	void onPlayButton();
public slots:
	void onFrameChange(QImage qImage);
};
