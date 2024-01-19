#include "MFPMainWindow.h"

#include <QFileDialog>

#include "QMessageBox"
#include "QDir"
#include "QPluginLoader"
#include <qstandarditemmodel.h>

MFPMainWindow::MFPMainWindow(QWidget* parent)
	: QMainWindow(parent) {
	ui.setupUi(this);
	ItemModel = new QStandardItemModel(this);
	actionDelete = new QAction(tr("Delete"), this);
	actionClear = new QAction(tr("Clear"), this);
	actionSetting = new QAction(tr("Settings"), this);
	menu = new QMenu(this);
	settingsDialog = new QDialog(this);
	settingsDialog->setModal(true);
	settingsUi.setupUi(settingsDialog);
	ui.settingsMenu->addAction(actionSetting);
	menu->addAction(actionClear);
	menu->addAction(actionDelete);
	ui.historyListView->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
	loadStyleSheet("MFPStyleSheet.qss");
	connect(ui.historyListView,SIGNAL(doubleClicked(QModelIndex)), this,SLOT(onDoubleClicked(QModelIndex)));
	connect(ui.openfileButton, SIGNAL(clicked()), this, SLOT(onOpenFileButton()));
	connect(actionDelete, SIGNAL(triggered()), this, SLOT(onActionDelete()));
	connect(actionClear, SIGNAL(triggered()), this, SLOT(onActionClear()));
	connect(actionSetting, SIGNAL(triggered()), this, SLOT(onActionSetting()));
	connect(settingsUi.hwDecodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onSettingsChanged()));
	connect(settingsUi.frameSpinBox, SIGNAL(editingFinished()), this, SLOT(onSettingsChanged()));
	connect(settingsUi.filePathlineEdit, SIGNAL(editingFinished()), this, SLOT(onSettingsChanged()));
	connect(settingsUi.openDirButton, SIGNAL(clicked()), this, SLOT(onOpenDirButton()));
	connect(ui.historyListView, SIGNAL(customContextMenuRequested(const QPoint&)), this,
	        SLOT(onListViewRightClickRequest(const QPoint&)));
}

MFPMainWindow::~MFPMainWindow() {
}

void MFPMainWindow::addPluginWidget(QWidget* widget) {
	ui.verticalLayout->addWidget(widget);
}

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

void MFPMainWindow::loadStyleSheet(const QString fileName)
{
	QFile file(fileName);
	if (file.open(QFile::ReadOnly))
	{
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
}

void MFPMainWindow::onDoubleClicked(const QModelIndex index) {
	emit play(index.row());
}
