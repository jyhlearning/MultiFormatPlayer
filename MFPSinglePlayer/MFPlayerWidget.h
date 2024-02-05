#pragma once

#include <QWidget>
#include "ui_MFPlayerWidget.h"
#include "ui_MFPInfomation.h"
#include "ui_MFPSettings.h"
#include "ui_MFPExport.h"
#include "MFPExportSettings.h"
#include "QFileDialog"
#include "QProgressDialog"
#include "QTimer"

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
	void setInformationDialog(const informaion& info) const;
	void setExportDialogResolution(const QStringList& list) const;
	void setExportDialogVideoBitRates(const QStringList& list) const;
	void setExportDialogAudioBitRates(const QStringList& list) const;
	void setExportDialogFormat(const QStringList& list) const;
	void setExprotDialogDefaultUrl(const QString s) const;
	void setExportVideoSettings(const settings& s) const;
	void setTimeSlider(const qint64 pts) const;
signals:
	void play(WidgetStete::statement sig);
	void progress(qint64 msec);
	void speed(double s);
	void stop();
	void volume(int v);
	void exports(settings s);
	void cancel();
	void fullScreen(bool state);

private:
	Ui::MFPlayerWidgetClass widgetUi;
	Ui::infomationDialog infomationDialogUi;
	Ui::settingsDialog settingsUi;
	Ui::exportDialog exportUi;
	QDialog *infomationDialog, *settingsDialog, *exportDialog;
	QFileDialog* fileOpenDialog;
	QProgressDialog* progressDialog;
	QTimer* timer;
	bool isFullScreen;
	void addExportItem(QComboBox* combox, const QString& text) const;
	void loadStyleSheet(const QString fileName);
	void mouseMoveEvent(QMouseEvent* event) override;

private slots:
	void onPlayButton();
	void onNextFrameButton();
	void onLastFrameButton();
	void onForwardButton();
	void onBackwardButton();
	void onInformationButton() const;
	void onResetButton() const;
	void onOutputButton() const;
	void onExportButton();
	void onOpenFileButton();
	void onFullScreenButton();

	void onCurrentIndexChanged(int c);

	void onSliderReleased();
	void onSliderPressed();
	void onSliderMoved();
	void onSettingsButton();
	void onBrightnessSlider();
	void onContrastSlider();
	void onSaturationSlider();

	void onTimerOut();

public slots:
	void onFrameChange(const QImage qImage) const;
	void onProgressChange(const qint64 sec) const;
	void setSliderRange(const qint64 min, const qint64 max) const;
	void setForwardLable(qint64 msec) const;
	void setBackwardLable(qint64 msec) const;
	void onProgress(qint64 p);
	void onCancel();
	void onError(const QString title, const QString info);
	void onChangeButton(QString qString);
	void onloadInitPic() const;
};
