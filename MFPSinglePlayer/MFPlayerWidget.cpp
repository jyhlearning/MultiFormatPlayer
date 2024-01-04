#include "MFPlayerWidget.h"
#include <QCloseEvent>

#include "QTime"

MFPlayerWidget::MFPlayerWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);
	connect(ui.playButton,SIGNAL(clicked()), this,SLOT(onPlayButton()));
	connect(ui.nextFrameButton,SIGNAL(clicked()),this,SLOT(onNextFrameButton()));
	connect(ui.lastFrameButton, SIGNAL(clicked()), this, SLOT(onLastFrameButton()));
	connect(ui.timeSlider,SIGNAL(sliderMoved(int)),this,SLOT(onSliderMoved(int)));
	connect(ui.speedComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
	ui.speedComboBox->addItem("0.5");
	ui.speedComboBox->addItem("1");
	ui.speedComboBox->addItem("2");
	ui.speedComboBox->addItem("4");
	ui.speedComboBox->setCurrentIndex(1);

}

MFPlayerWidget::~MFPlayerWidget() {
}

void MFPlayerWidget::changeButton(QString qString) { ui.playButton->setText(qString); }

void MFPlayerWidget::onNextFrameButton()
{
	emit play(WidgetStete::NEXTFRAME);
}

void MFPlayerWidget::onLastFrameButton()
{
	emit play(WidgetStete::LASTFRAME);
}

void MFPlayerWidget::onSliderMoved(int v)
{
	emit progress(v);
}

void MFPlayerWidget::onCurrentIndexChanged(int c)
{
	double temp = 1;
	switch (c) {
	case 0:
		temp = 0.5;
		break;
	case 2:
		temp = 2;
		break;
	case 3:
		temp = 4;
		break;
		default: 
			temp = 1;
	}
	emit speed(temp);
}

void MFPlayerWidget::onProgressChange(const qint64 sec,const qint64 totalTime) {
	if(totalTime!=ui.timeSlider->maximum()) {
		ui.timeSlider->setRange(0, totalTime);
	}
	//ui.timeSlider->blockSignals(true);
	ui.timeSlider->setValue(sec);
	//ui.timeSlider->blockSignals(false);
}


void MFPlayerWidget::onPlayButton() {
	emit play(WidgetStete::PLAY);
}

void MFPlayerWidget::onFrameChange(QImage qImage) {
	int label_width = ui.videoLabel->width();
	int label_height = ui.videoLabel->height();

	// 缩放图片以适应QLabel，保持纵横比
	qImage = qImage.scaled(label_width, label_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);


	ui.videoLabel->setPixmap(QPixmap::fromImage(qImage));
}
