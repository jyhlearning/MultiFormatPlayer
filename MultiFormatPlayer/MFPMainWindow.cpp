﻿#include "MFPMainWindow.h"

#include <QFileDialog>

#include "QMessageBox"
#include "QDir"
#include "QPluginLoader"
#include <qstandarditemmodel.h>
#include "QTranslator"

MFPMainWindow::MFPMainWindow(QWidget* parent)
	: QMainWindow(parent) {
	ui.setupUi(this);
	ItemModel = new QStandardItemModel(this);
	menu = new QMenu(this);
	settingsDialog = new QDialog(this);
	actionDelete = new QAction(tr("删除"), this);
	actionClear = new QAction(tr("清空"), this);
	actionSetting = new QAction(tr("设置"), this);
	settingsDialog->setModal(true);
	settingsUi.setupUi(settingsDialog);
	ui.settingsMenu->addAction(actionSetting);
	menu->addAction(actionClear);
	menu->addAction(actionDelete);
	ui.historyListView->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
	loadStyleSheet(":/MFPStyleSheet.qss");
	setWindowIcon(QIcon(":/logo.ico"));

	connect(ui.historyListView,SIGNAL(doubleClicked(QModelIndex)), this,SLOT(onDoubleClicked(QModelIndex)));
	connect(ui.openfileButton, SIGNAL(clicked()), this, SLOT(onOpenFileButton()));
	connect(actionDelete, SIGNAL(triggered()), this, SLOT(onActionDelete()));
	connect(actionClear, SIGNAL(triggered()), this, SLOT(onActionClear()));
	connect(actionSetting, SIGNAL(triggered()), this, SLOT(onActionSetting()));
	connect(settingsUi.hwDecodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onSettingsChanged()));
	connect(settingsUi.frameSpinBox, SIGNAL(editingFinished()), this, SLOT(onSettingsChanged()));
	connect(settingsUi.filePathlineEdit, SIGNAL(editingFinished()), this, SLOT(onSettingsChanged()));
	connect(settingsUi.openDirButton, SIGNAL(clicked()), this, SLOT(onOpenDirButton()));
	connect(settingsUi.languageComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSettingsChanged()));
	connect(ui.historyListView, SIGNAL(customContextMenuRequested(const QPoint&)), this,
	        SLOT(onListViewRightClickRequest(const QPoint&)));
}

MFPMainWindow::~MFPMainWindow() {
}

void MFPMainWindow::addPluginWidget(QWidget* widget) { ui.verticalLayout->addWidget(widget); }

void MFPMainWindow::loadHistory() {
	for (int i = 0; i < history->size(); i++) {
		ItemModel->appendRow(new QStandardItem(history->at(i).toString().section('/', -1)));
	}
	ui.historyListView->setModel(ItemModel);
}

void MFPMainWindow::read(QJsonObject& obj) {
	this->obj = &obj;
	setFilter(obj.value("singlePlayer").toObject());
	setSettings(obj.value("singlePlayer").toObject());
	setLanguage(obj);
}

void MFPMainWindow::setHistory(QJsonArray* array) { history = array; }

void MFPMainWindow::setFilter(const QJsonObject& obj) {
	QJsonArray temp = obj.value("format").toArray();

	for (auto s : temp) { filter += " *." + s.toString(); }
}

void MFPMainWindow::setSettings(const QJsonObject& obj) {
	settingsUi.hwDecodeCheckBox->blockSignals(true);
	settingsUi.frameSpinBox->blockSignals(true);
	settingsUi.filePathlineEdit->blockSignals(true);
	settingsUi.hwDecodeCheckBox->setCheckState(obj.value("hWDecode").toBool() ? Qt::Checked : Qt::Unchecked);
	settingsUi.frameSpinBox->setValue(obj.value("preLoadFrames").toInt());
	settingsUi.filePathlineEdit->setText(obj.value("outputUrl").toString());
	settingsUi.hwDecodeCheckBox->blockSignals(false);
	settingsUi.frameSpinBox->blockSignals(false);
	settingsUi.filePathlineEdit->blockSignals(false);
}

void MFPMainWindow::setLanguage(const QJsonObject& obj) {
	settingsUi.languageComboBox->blockSignals(true);
	const QJsonArray temp = obj["languages"].toArray();
	const QString defaultLanguage = obj["defaultLanguage"].toString();
	int defaultIndex = 0,i=0;
	for(auto a:temp) {
		QJsonObject b= a.toObject();
		settingsUi.languageComboBox->addItem(b["name"].toString());
		languages.append(b["suffix"].toString());
		if (b["suffix"].toString() == defaultLanguage)
			defaultIndex = i;
		i++;
	}
	settingsUi.languageComboBox->setCurrentIndex(defaultIndex);
	settingsUi.languageComboBox->blockSignals(false);
}

void MFPMainWindow::fullScreen() {
	ui.menuBar->hide();
	hideLayout(ui.rightLayout);
	ui.centralWidget->layout()->setContentsMargins(0, 0, 0, 0);
	this->setWindowFlags(Qt::FramelessWindowHint);
	this->showFullScreen();
}

void MFPMainWindow::window() {
	ui.menuBar->show();
	showLayout(ui.rightLayout);
	ui.centralWidget->layout()->setContentsMargins(11, 11, 11, 11);
	this->setWindowState(Qt::WindowActive);
	this->setWindowFlags(Qt::Window);
	this->showNormal();
}

void MFPMainWindow::hideLayout(const QLayout* layout) {
	for (int i = 0; i < layout->count(); i++) {
		QLayoutItem* item = layout->itemAt(i);
		if (item->widget()) { item->widget()->hide(); }
		else if (item->layout()) { hideLayout(item->layout()); }
	}
}

void MFPMainWindow::showLayout(const QLayout* layout) {
	for (int i = 0; i < layout->count(); i++) {
		QLayoutItem* item = layout->itemAt(i);
		if (item->widget()) { item->widget()->show(); }
		else if (item->layout()) { showLayout(item->layout()); }
	}
}

void MFPMainWindow::loadStyleSheet(const QString fileName) {
	QFile file(fileName);
	if (file.open(QFile::ReadOnly)) {
		this->setStyleSheet(file.readAll());
		file.close();
	}
}

void MFPMainWindow::onOpenFileButton() {
	const QString url = QFileDialog::getOpenFileUrl(this, QStringLiteral("选择路径"), QString("../../"), filter,
	                                                nullptr, QFileDialog::DontUseCustomDirectoryIcons).toLocalFile();
	if (url.size() == 0)return;
	ui.pathEdit->setText(url);
	for (int i = 0; i < history->size(); i++) {
		QString a = history->at(i).toString();
		if (history->at(i).toString() == url) {
			emit play(i);
			return;
		}
	}
	ItemModel->appendRow(new QStandardItem(url.section('/', -1)));
	history->append(url);
	emit play(history->size() - 1);
}

void MFPMainWindow::onOpenDirButton() {
	settingsUi.filePathlineEdit->setText(
		QFileDialog::getExistingDirectory(this, tr("选择文件保存路径"), "../../", QFileDialog::ShowDirsOnly));
	onSettingsChanged();
}

void MFPMainWindow::onActionDelete() {
	QModelIndex index = ui.historyListView->selectionModel()->currentIndex();
	ItemModel->removeRow(index.row());
	history->removeAt(index.row());
}

void MFPMainWindow::onActionClear() {
	ItemModel->clear();
	while (!history->isEmpty())
		history->removeFirst();
}

void MFPMainWindow::onActionSetting() { settingsDialog->show(); }

void MFPMainWindow::onListViewRightClickRequest(const QPoint& p) { menu->exec(QCursor::pos()); }

void MFPMainWindow::onSettingsChanged() {
	QJsonObject a = (*obj)["singlePlayer"].toObject();
	a["hWDecode"] = settingsUi.hwDecodeCheckBox->isChecked();
	a["outputUrl"] = settingsUi.filePathlineEdit->text();
	a["preLoadFrames"] = settingsUi.frameSpinBox->value();
	(*obj)["singlePlayer"] = a;
	(*obj)["defaultLanguage"] = languages[settingsUi.languageComboBox->currentIndex()];
}

void MFPMainWindow::onDoubleClicked(const QModelIndex index) {
	emit play(index.row());
}
