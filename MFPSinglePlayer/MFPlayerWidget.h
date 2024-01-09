#pragma once

#include <QWidget>
#include "ui_MFPlayerWidget.h"
#include "ui_MFPInfomation.h"
namespace WidgetStete {
	enum statement {
		PLAY,
		NEXTFRAME,
		LASTFRAME
	};
}

struct informaion {
	std::pair<int, int> resolution;
	qint64 length;
	int frameRate;
	int channels;
};


class MFPlayerWidget : public QWidget {
	Q_OBJECT

public:
	MFPlayerWidget(QWidget* parent = nullptr);
	~MFPlayerWidget();
	void changeButton(QString qString);
	void setInformationDialog(const informaion &info) const;
signals:
	void play(WidgetStete::statement sig);
	void progress(qint64 msec);
	void speed(double s);
	void stop();
	void volume(int v);

private:
	Ui::MFPlayerWidgetClass widgetUi;
	Ui::infomationDialog dialogUi;
	QDialog *dialog;
private slots:
	void onPlayButton();
	void onNextFrameButton();
	void onLastFrameButton();
	void onForwardButton();
	void onBackwardButton();
	void onInformationButton();
	void onSliderReleased();
	void onSliderPressed();
	void onCurrentIndexChanged(int c);
	void onSliderMoved();
public slots:
	void onFrameChange(QImage qImage) const;
	void onProgressChange(const qint64 sec) const;
	void setSliderRange(const qint64 min, const qint64 max) const;
	void setForwardLable(qint64 msec) const;
	void setBackwardLable(qint64 msec) const;
};
