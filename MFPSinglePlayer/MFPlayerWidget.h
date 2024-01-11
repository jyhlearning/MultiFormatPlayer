#pragma once

#include <QWidget>
#include "ui_MFPlayerWidget.h"
#include "ui_MFPInfomation.h"
#include "ui_MFPSettings.h"
#include "ui_MFPExport.h"
#include "MFPExportSettings.h"
#include "QFileDialog"
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
	void setExportDialogResolution(const QStringList& list)const;
	void setExportDialogVideoBitRates(const QStringList& list)const;
	void setExportDialogAudioBitRates(const QStringList& list)const;
	void setExportDefaultSettings(const settings &s) const;
signals:
	void play(WidgetStete::statement sig);
	void progress(qint64 msec);
	void speed(double s);
	void stop();
	void volume(int v);
	void exports(settings s);
private:
	Ui::MFPlayerWidgetClass widgetUi;
	Ui::infomationDialog infomationDialogUi;
	Ui::settingsDialog settingsUi;
	Ui::exportDialog exportUi;
	QDialog* infomationDialog, * settingsDialog, * exportDialog;
	QFileDialog *fileOpenDialog;
	void addExportItem(QComboBox* combox,const QString & text) const;
private slots:
	void onPlayButton();
	void onNextFrameButton();
	void onLastFrameButton();
	void onForwardButton();
	void onBackwardButton();
	void onInformationButton();
	void onResetButton();
	void onOutputButton();
	void onExportButton();
	void onOpenFileButton();

	void onCurrentIndexChanged(int c);

	void onSliderReleased();
	void onSliderPressed();
	void onSliderMoved();
	void onSettingsButton();
	void onBrightnessSlider();
	void onContrastSlider();
	void onSaturationSlider();

	void onFileChoose();
public slots:
	void onFrameChange(const QImage qImage) const;
	void onProgressChange(const qint64 sec) const;
	void setSliderRange(const qint64 min, const qint64 max) const;
	void setForwardLable(qint64 msec) const;
	void setBackwardLable(qint64 msec) const;
};
