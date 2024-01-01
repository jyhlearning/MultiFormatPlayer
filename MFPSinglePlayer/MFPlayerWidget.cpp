#include "MFPlayerWidget.h"
#include <QCloseEvent>

#include "QTime"
MFPlayerWidget::MFPlayerWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);
	connect(ui.playButton,SIGNAL(clicked()), this,SLOT(onPlayButton()));
}

MFPlayerWidget::~MFPlayerWidget() {
}

void MFPlayerWidget::changeButton(QString qString) { ui.playButton->setText(qString); }


void MFPlayerWidget::onPlayButton() {
	emit play();
}

void MFPlayerWidget::onFrameChange(QImage qImage) {

	int label_width = ui.videoLabel->width();
	int label_height = ui.videoLabel->height();

	// 缩放图片以适应QLabel，保持纵横比
	qImage = qImage.scaled(label_width,label_height ,Qt::KeepAspectRatio,Qt::SmoothTransformation);
	

	ui.videoLabel->setPixmap(QPixmap::fromImage(qImage));

}
