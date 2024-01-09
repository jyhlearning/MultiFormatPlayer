#include "MFPlayerWidget.h"
#include <QCloseEvent>

#include "QTime"

MFPlayerWidget::MFPlayerWidget(QWidget* parent)
	: QWidget(parent) {
	widgetUi.setupUi(this);
	dialog = new QDialog;
	dialogUi.setupUi(dialog);
	connect(widgetUi.playButton,SIGNAL(clicked()), this,SLOT(onPlayButton()));
	connect(widgetUi.nextFrameButton,SIGNAL(clicked()), this,SLOT(onNextFrameButton()));
	connect(widgetUi.lastFrameButton, SIGNAL(clicked()), this, SLOT(onLastFrameButton()));
	connect(widgetUi.forwardButton, SIGNAL(clicked()), this, SLOT(onForwardButton()));
	connect(widgetUi.backwardButton, SIGNAL(clicked()), this, SLOT(onBackwardButton()));

	connect(widgetUi.timeSlider, SIGNAL(press()), this, SLOT(onSliderPressed()));
	connect(widgetUi.timeSlider,SIGNAL(release()), this,SLOT(onSliderReleased()));

	connect(widgetUi.volumeSlider, SIGNAL(release()), this, SLOT(onSliderMoved()));
	connect(widgetUi.volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(onSliderMoved()));
	connect(widgetUi.speedComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
	connect(widgetUi.infomationButton,SIGNAL(clicked()), this, SLOT(onInformationButton()));
	widgetUi.speedComboBox->addItem("0.5");
	widgetUi.speedComboBox->addItem("1");
	widgetUi.speedComboBox->addItem("2");
	widgetUi.speedComboBox->addItem("4");
	widgetUi.speedComboBox->setCurrentIndex(1);
}

MFPlayerWidget::~MFPlayerWidget() {
	delete dialog;
}

void MFPlayerWidget::changeButton(QString qString) { widgetUi.playButton->setText(qString); }


void MFPlayerWidget::setInformationDialog(const informaion& info) const
{
	dialogUi.frameRate->setText(QString::number(info.frameRate));
	dialogUi.channels->setText(QString::number(info.channels));
	const QTime time = QTime::fromMSecsSinceStartOfDay(info.length);
	dialogUi.length->setText(QString("%1:%2:%3")
		.arg(time.hour(), 2, 10, QChar('0'))
		.arg(time.minute(), 2, 10, QChar('0'))
		.arg(time.second(), 2, 10, QChar('0')));
	dialogUi.resolution->setText(QString::number(info.resolution.first)+" × " + QString::number(info.resolution.second));
}

void MFPlayerWidget::onNextFrameButton() {
	emit play(WidgetStete::NEXTFRAME);
}

void MFPlayerWidget::onLastFrameButton() {
	emit play(WidgetStete::LASTFRAME);
}

void MFPlayerWidget::onForwardButton() {
	emit stop();
	int value = widgetUi.timeSlider->value() + 10 * 1000;
	value = value > widgetUi.timeSlider->maximum() ? widgetUi.timeSlider->maximum() : value;
	emit progress(value);
}

void MFPlayerWidget::onBackwardButton() {
	emit stop();
	int value = widgetUi.timeSlider->value() - 10 * 1000;
	value = value < 0 ? 0 : value;
	emit progress(value);
}

void MFPlayerWidget::onInformationButton()
{
	dialog->show();
}

void MFPlayerWidget::onSliderReleased() {
	int a = widgetUi.timeSlider->value();
	emit progress(widgetUi.timeSlider->value());
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

void MFPlayerWidget::onSliderMoved()
{
	emit volume(widgetUi.volumeSlider->value());
	widgetUi.volumeLabel->setText(QString::number(widgetUi.volumeSlider->value()));
}

void MFPlayerWidget::onProgressChange(const qint64 sec) const {
	widgetUi.timeSlider->setValue(sec);
	setForwardLable(sec);
	setBackwardLable(widgetUi.timeSlider->maximum()-sec);
}

void MFPlayerWidget::setSliderRange(const qint64 min, const qint64 max) const {
	widgetUi.timeSlider->setRange(min, max);
	
}

void MFPlayerWidget::setForwardLable(qint64 msec) const
{
	msec = msec < 0 ? 0 : msec;
	const QTime time = QTime::fromMSecsSinceStartOfDay(msec);
	widgetUi.forwardLabel->setText(QString("%1:%2:%3")
		.arg(time.hour(), 2, 10, QChar('0'))
		.arg(time.minute(), 2, 10, QChar('0'))
		.arg(time.second(), 2, 10, QChar('0')));
}

void MFPlayerWidget::setBackwardLable(qint64 msec) const
{
	msec = msec < 0 ? 0 : msec;
	const QTime time = QTime::fromMSecsSinceStartOfDay(msec);
	widgetUi.backwardLabel->setText(QString("%1:%2:%3")
		.arg(time.hour(), 2, 10, QChar('0'))
		.arg(time.minute(), 2, 10, QChar('0'))
		.arg(time.second(), 2, 10, QChar('0')));
}


void MFPlayerWidget::onPlayButton() {
	emit play(WidgetStete::PLAY);
}

void MFPlayerWidget::onFrameChange(QImage qImage) const {
	
	//int label_width = widgetUi.videoLabel->width();
	//int label_height = widgetUi.videoLabel->height();
	//// 缩放图片以适应QLabel，保持纵横比
	//qImage = qImage.scaled(label_width, label_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	//widgetUi.videoLabel->setPixmap(QPixmap::fromImage(qImage));

	widgetUi.videoWidget->setImage(qImage);
}
