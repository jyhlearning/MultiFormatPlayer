#include "MFPlayerWidget.h"
#include <QCloseEvent>

#include "QTime"

MFPlayerWidget::MFPlayerWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);
	connect(ui.playButton,SIGNAL(clicked()), this,SLOT(onPlayButton()));
	connect(ui.nextFrameButton,SIGNAL(clicked()),this,SLOT(onNextFrameButton()));
	connect(ui.timeSlider,SIGNAL(sliderMoved(int)),this,SLOT(onSliderMoved(int)));
}

MFPlayerWidget::~MFPlayerWidget() {
}

void MFPlayerWidget::changeButton(QString qString) { ui.playButton->setText(qString); }

void MFPlayerWidget::onNextFrameButton()
{
	emit play(WidgetStete::NEXTFRAME);
}

void MFPlayerWidget::onSliderMoved(int v)
{
	emit progress(v);
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
