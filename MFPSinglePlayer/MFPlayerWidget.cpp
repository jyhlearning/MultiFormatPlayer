#include "MFPlayerWidget.h"
#include <QCloseEvent>

#include "QTime"

MFPlayerWidget::MFPlayerWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);
	connect(ui.playButton,SIGNAL(clicked()), this,SLOT(onPlayButton()));
	connect(ui.nextFrameButton,SIGNAL(clicked()), this,SLOT(onNextFrameButton()));
	connect(ui.lastFrameButton, SIGNAL(clicked()), this, SLOT(onLastFrameButton()));
	connect(ui.forwardButton, SIGNAL(clicked()), this, SLOT(onForwardButton()));
	connect(ui.backwardButton, SIGNAL(clicked()), this, SLOT(onBackwardButton()));

	connect(ui.timeSlider, SIGNAL(clicked()), this, SLOT(onSliderPressed()));
	//connect(ui.timeSlider,SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
	connect(ui.timeSlider,SIGNAL(sliderReleased()), this,SLOT(onSliderReleased()));
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

void MFPlayerWidget::onNextFrameButton() {
	emit play(WidgetStete::NEXTFRAME);
}

void MFPlayerWidget::onLastFrameButton() {
	emit play(WidgetStete::LASTFRAME);
}

void MFPlayerWidget::onForwardButton() {
	emit stop();
	int value = ui.timeSlider->value() + 10 * 1000;
	value = value > ui.timeSlider->maximum() ? ui.timeSlider->maximum() : value;
	emit progress(value);
}

void MFPlayerWidget::onBackwardButton() {
	emit stop();
	int value = ui.timeSlider->value() - 10 * 1000;
	value = value < 0 ? 0 : value;
	emit progress(value);
}

void MFPlayerWidget::onSliderReleased() {
	emit progress(ui.timeSlider->value());
}

void MFPlayerWidget::onSliderPressed() {
	emit stop();
}


void MFPlayerWidget::onCurrentIndexChanged(int c) {
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

void MFPlayerWidget::onProgressChange(const qint64 sec) const {
	ui.timeSlider->setValue(sec);
	setForwardLable(sec);
	setBackwardLable(ui.timeSlider->maximum()-sec);
}

void MFPlayerWidget::setSliderRange(const qint64 min, const qint64 max) const {
	ui.timeSlider->setRange(min, max);
	
}

void MFPlayerWidget::setForwardLable(qint64 msec) const
{
	msec = msec < 0 ? 0 : msec;
	const QTime time = QTime::fromMSecsSinceStartOfDay(msec);
	ui.forwardLabel->setText(QString("%1:%2:%3")
		.arg(time.hour(), 2, 10, QChar('0'))
		.arg(time.minute(), 2, 10, QChar('0'))
		.arg(time.second(), 2, 10, QChar('0')));
}

void MFPlayerWidget::setBackwardLable(qint64 msec) const
{
	msec = msec < 0 ? 0 : msec;
	const QTime time = QTime::fromMSecsSinceStartOfDay(msec);
	ui.backwardLabel->setText(QString("%1:%2:%3")
		.arg(time.hour(), 2, 10, QChar('0'))
		.arg(time.minute(), 2, 10, QChar('0'))
		.arg(time.second(), 2, 10, QChar('0')));
}


void MFPlayerWidget::onPlayButton() {
	emit play(WidgetStete::PLAY);
}

void MFPlayerWidget::onFrameChange(QImage qImage) const {
	int label_width = ui.videoLabel->width();
	int label_height = ui.videoLabel->height();

	// 缩放图片以适应QLabel，保持纵横比
	qImage = qImage.scaled(label_width, label_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);


	ui.videoLabel->setPixmap(QPixmap::fromImage(qImage));
}
