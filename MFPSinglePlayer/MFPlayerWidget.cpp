#include "MFPlayerWidget.h"
#include <QCloseEvent>
#include <QMessageBox>

#include "QTime"

MFPlayerWidget::MFPlayerWidget(QWidget* parent)
	: QWidget(parent) {
	widgetUi.setupUi(this);

	setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	hide();

	infomationDialog = new QDialog(this);
	infomationDialogUi.setupUi(infomationDialog);
	settingsDialog = new QDialog(this);
	settingsUi.setupUi(settingsDialog);
	exportDialog = new QDialog(this);
	exportDialog->setModal(true);
	exportUi.setupUi(exportDialog);
	fileOpenDialog = new QFileDialog(exportDialog);
	fileOpenDialog->setModal(true);
	fileOpenDialog->setFileMode(QFileDialog::Directory);
	progressDialog = new QProgressDialog(exportDialog);
	progressDialog->close();
	progressDialog->setModal(true);
	progressDialog->setWindowTitle(tr("请等待"));
	progressDialog->setLabelText(tr("导出中..."));
	progressDialog->setCancelButtonText(tr("终止"));
	connect(widgetUi.playButton,SIGNAL(clicked()), this,SLOT(onPlayButton()));
	connect(widgetUi.nextFrameButton,SIGNAL(clicked()), this,SLOT(onNextFrameButton()));
	connect(widgetUi.lastFrameButton, SIGNAL(clicked()), this, SLOT(onLastFrameButton()));
	connect(widgetUi.forwardButton, SIGNAL(clicked()), this, SLOT(onForwardButton()));
	connect(widgetUi.backwardButton, SIGNAL(clicked()), this, SLOT(onBackwardButton()));
	connect(widgetUi.infomationButton,SIGNAL(clicked()), this, SLOT(onInformationButton()));
	connect(widgetUi.settingButton, SIGNAL(clicked()), this, SLOT(onSettingsButton()));
	connect(widgetUi.outputButton,SIGNAL(clicked()), this,SLOT(onOutputButton()));

	connect(widgetUi.timeSlider, SIGNAL(press()), this, SLOT(onSliderPressed()));
	connect(widgetUi.timeSlider,SIGNAL(release()), this,SLOT(onSliderReleased()));

	connect(widgetUi.volumeSlider, SIGNAL(release()), this, SLOT(onSliderMoved()));
	connect(widgetUi.volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(onSliderMoved()));
	connect(widgetUi.speedComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

	connect(settingsUi.brightnessSlider,SIGNAL(sliderMoved(int)), this,SLOT(onBrightnessSlider()));
	connect(settingsUi.brightnessSlider, SIGNAL(release()), this, SLOT(onBrightnessSlider()));
	connect(settingsUi.contrastSlider, SIGNAL(sliderMoved(int)), this, SLOT(onContrastSlider()));
	connect(settingsUi.contrastSlider, SIGNAL(release()), this, SLOT(onContrastSlider()));
	connect(settingsUi.saturationSlider, SIGNAL(sliderMoved(int)), this, SLOT(onSaturationSlider()));
	connect(settingsUi.saturationSlider, SIGNAL(release()), this, SLOT(onSaturationSlider()));

	connect(settingsUi.resetButton,SIGNAL(clicked()), this,SLOT(onResetButton()));

	connect(exportUi.exportButton,SIGNAL(clicked()), this,SLOT(onExportButton()));
	connect(exportUi.openFileButton,SIGNAL(clicked()), this,SLOT(onOpenFileButton()));

	connect(progressDialog, SIGNAL(canceled()), this, SLOT(onCancel()));
	widgetUi.speedComboBox->addItem("0.5");
	widgetUi.speedComboBox->addItem("1");
	widgetUi.speedComboBox->addItem("2");
	widgetUi.speedComboBox->addItem("4");
	widgetUi.speedComboBox->setCurrentIndex(1);
}

MFPlayerWidget::~MFPlayerWidget() {
	delete infomationDialog;
	delete settingsDialog;
	delete exportDialog;
}

void MFPlayerWidget::changeButton(QString qString) { widgetUi.playButton->setText(qString); }


void MFPlayerWidget::setInformationDialog(const informaion& info) const {
	infomationDialogUi.frameRate->setText(QString::number(info.frameRate));
	infomationDialogUi.channels->setText(QString::number(info.channels));
	const QTime time = QTime::fromMSecsSinceStartOfDay(info.length);
	infomationDialogUi.length->setText(QString("%1:%2:%3")
	                                   .arg(time.hour(), 2, 10, QChar('0'))
	                                   .arg(time.minute(), 2, 10, QChar('0'))
	                                   .arg(time.second(), 2, 10, QChar('0')));
	infomationDialogUi.resolution->setText(
		QString::number(info.resolution.first) + " × " + QString::number(info.resolution.second));
}

void MFPlayerWidget::setExportDialogResolution(const QStringList& list) const {
	exportUi.resolutionComboBox->clear();
	exportUi.resolutionComboBox->addItems(list);
}

void MFPlayerWidget::setExportDialogVideoBitRates(const QStringList& list) const {
	exportUi.videoBitRatecomboBox->clear();
	exportUi.videoBitRatecomboBox->addItems(list);
}

void MFPlayerWidget::setExportDialogAudioBitRates(const QStringList& list) const {
	exportUi.audioBitRateComboBox->clear();
	exportUi.audioBitRateComboBox->addItems(list);
}

void MFPlayerWidget::setExportDialogFormat(const QStringList& list) const
{
	exportUi.formatComboBox->clear();
	exportUi.formatComboBox->addItems(list);
}

void MFPlayerWidget::setExprotDialogDefaultUrl(const QString s) const
{
	exportUi.fileEdit->setText(s);
}

void MFPlayerWidget::setExportVideoSettings(const settings& s) const {
	exportUi.onlyVideoCheckBox->setChecked(!s.closeVideo);
	exportUi.onlyAudioCheckBox->setChecked(!s.closeAudio);
	exportUi.startTimeEdit->setTime(QTime(0, 0, 0).addMSecs(s.startPts));
	exportUi.endTimeEdit->setTime(QTime(0, 0, 0).addMSecs(s.endPts));
	addExportItem(exportUi.audioBitRateComboBox, QString::number(s.audioBitRate));
	addExportItem(exportUi.videoBitRatecomboBox, QString::number(s.videoBitRate));
	addExportItem(exportUi.resolutionComboBox, QString::number(s.outWidth) + "*" + QString::number(s.outHeight));
	for (int i = 0; i < exportUi.resolutionComboBox->count(); i++) {
		QStringList resolutions = exportUi.resolutionComboBox->itemText(i).split('*');
		if (resolutions.size() == 2 && s.outWidth < resolutions[0].toInt() && s.outHeight < resolutions[1].toInt()) {
			exportUi.resolutionComboBox->setItemData(i, 0, Qt::UserRole - 1);
		}
	}
}

void MFPlayerWidget::addExportItem(QComboBox* combox, const QString& text) const {
	bool flag = false;
	for (int i = 0; i < combox->count(); i++) {
		if (combox->itemText(i) == text) {
			flag = true;
			combox->setCurrentIndex(i);
			break;
		}
	}
	if (!flag) {
		combox->addItem(text);
		combox->setCurrentIndex(combox->count() - 1);
	}
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

void MFPlayerWidget::onInformationButton() { infomationDialog->show(); }

void MFPlayerWidget::onResetButton() {
	settingsUi.brightnessSlider->setValue(50);
	settingsUi.contrastSlider->setValue(50);
	settingsUi.saturationSlider->setValue(50);

	settingsUi.brightnessLabel->setText("50");
	settingsUi.contrastLabel->setText("50");
	settingsUi.saturationLabel->setText("50");

	widgetUi.videoWidget->setBrightness(0);
	widgetUi.videoWidget->setSaturation(1);
	widgetUi.videoWidget->setContrast(1);
}

void MFPlayerWidget::onOutputButton() { exportDialog->show(); }

void MFPlayerWidget::onExportButton() {
	settings s;
	if (!(exportUi.fileEdit->text().size()&&exportUi.nameLineEdit->text().size())) {
		QMessageBox::critical(this, tr("error"), tr("请设置正确的文件名或文件路径"), QMessageBox::Discard);
		return;
	}
	s.URL = exportUi.fileEdit->text()+"/"+exportUi.nameLineEdit->text()+"."+exportUi.formatComboBox->currentText();
	s.closeAudio = !exportUi.onlyAudioCheckBox->isChecked();
	s.closeVideo = !exportUi.onlyVideoCheckBox->isChecked();
	s.startPts = exportUi.startTimeEdit->time().msecsSinceStartOfDay();
	s.endPts = exportUi.endTimeEdit->time().msecsSinceStartOfDay();
	s.audioBitRate = exportUi.audioBitRateComboBox->currentText().toLongLong();
	s.videoBitRate = exportUi.videoBitRatecomboBox->currentText().toLongLong();
	QStringList resolutions = exportUi.resolutionComboBox->currentText().split('*');
	if (resolutions.size() == 2) {
		progressDialog->setRange(s.startPts, s.endPts);
		progressDialog->show();
		s.outWidth = resolutions[0].toInt();
		s.outHeight = resolutions[1].toInt();
		emit exports(s);
	}
	else
		QMessageBox::critical(this, tr("error"), tr("请检查分辨率配置文件"), QMessageBox::Discard);
}

void MFPlayerWidget::onOpenFileButton() {
	exportUi.fileEdit->setText(
		QFileDialog::getExistingDirectory(this, tr("选择文件保存路径"), "../../", QFileDialog::ShowDirsOnly));
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

void MFPlayerWidget::onSliderMoved() {
	emit volume(widgetUi.volumeSlider->value());
	widgetUi.volumeLabel->setText(QString::number(widgetUi.volumeSlider->value()));
}

void MFPlayerWidget::onSettingsButton() { settingsDialog->show(); }

void MFPlayerWidget::onBrightnessSlider() {
	settingsUi.brightnessLabel->setText(QString::number(settingsUi.brightnessSlider->value()));
	widgetUi.videoWidget->setBrightness((settingsUi.brightnessSlider->value() - 50) * 1.0 / 50);
}

void MFPlayerWidget::onContrastSlider() {
	settingsUi.contrastLabel->setText(QString::number(settingsUi.contrastSlider->value()));
	widgetUi.videoWidget->setContrast(settingsUi.contrastSlider->value() * 2.0 / 100);
}

void MFPlayerWidget::onSaturationSlider() {
	settingsUi.saturationLabel->setText(QString::number(settingsUi.saturationSlider->value()));
	widgetUi.videoWidget->setSaturation(settingsUi.saturationSlider->value() * 2.0 / 100);
}

void MFPlayerWidget::onFileChoose() {
}

void MFPlayerWidget::onProgressChange(const qint64 sec) const {
	widgetUi.timeSlider->setValue(sec);
	setForwardLable(sec);
	setBackwardLable(widgetUi.timeSlider->maximum() - sec);
}

void MFPlayerWidget::setSliderRange(const qint64 min, const qint64 max) const {
	widgetUi.timeSlider->setRange(min, max);
}

void MFPlayerWidget::setForwardLable(qint64 msec) const {
	msec = msec < 0 ? 0 : msec;
	const QTime time = QTime::fromMSecsSinceStartOfDay(msec);
	widgetUi.forwardLabel->setText(QString("%1:%2:%3")
	                               .arg(time.hour(), 2, 10, QChar('0'))
	                               .arg(time.minute(), 2, 10, QChar('0'))
	                               .arg(time.second(), 2, 10, QChar('0')));
}

void MFPlayerWidget::setBackwardLable(qint64 msec) const {
	msec = msec < 0 ? 0 : msec;
	const QTime time = QTime::fromMSecsSinceStartOfDay(msec);
	widgetUi.backwardLabel->setText(QString("%1:%2:%3")
	                                .arg(time.hour(), 2, 10, QChar('0'))
	                                .arg(time.minute(), 2, 10, QChar('0'))
	                                .arg(time.second(), 2, 10, QChar('0')));
}

void MFPlayerWidget::onProgress(qint64 p) { progressDialog->setValue(p); }

void MFPlayerWidget::onCancel() {
	emit cancel();
}

void MFPlayerWidget::onError(const QString title, const QString info)
{
	QMessageBox::warning(this, title, info, QMessageBox::Close);
}

void MFPlayerWidget::onPlayButton() {
	emit play(WidgetStete::PLAY);
}

void MFPlayerWidget::onFrameChange(const QImage qImage) const {
	//int label_width = widgetUi.videoLabel->width();
	//int label_height = widgetUi.videoLabel->height();
	//// 缩放图片以适应QLabel，保持纵横比
	//qImage = qImage.scaled(label_width, label_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	//widgetUi.videoLabel->setPixmap(QPixmap::fromImage(qImage));

	widgetUi.videoWidget->setImage(qImage);
}
