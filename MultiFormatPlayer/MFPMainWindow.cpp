#include "MFPMainWindow.h"

#include <QFileDialog>

#include "QMessageBox"
#include "QDir"
#include "QPluginLoader"
#include <qstandarditemmodel.h>

MFPMainWindow::MFPMainWindow(QWidget* parent)
	: QMainWindow(parent) {
	ui.setupUi(this);
	connect(ui.historyListView,SIGNAL(doubleClicked(QModelIndex)), this,SLOT(onDoubleClicked(QModelIndex)));
	connect(ui.openfileButton, SIGNAL(clicked()), this, SLOT(onOpenFileButton()));
}

MFPMainWindow::~MFPMainWindow() {
}

void MFPMainWindow::addPluginWidget(QWidget* widget) { ui.verticalLayout->addWidget(widget); }

void MFPMainWindow::loadHistory(QJsonArray history) {
	QStandardItemModel* ItemModel = new QStandardItemModel(this);
	for (int i = 0; i < history.size(); i++) {
		ItemModel->appendRow(new QStandardItem(history[i].toString().section('/', -1)));
	}
	ui.historyListView->setModel(ItemModel);
}

void MFPMainWindow::onOpenFileButton()
{
	QUrl file_name = QFileDialog::getOpenFileUrl(this, QStringLiteral("选择路径"), QString("../../"), "*.mp4 *.avi", nullptr, QFileDialog::DontUseCustomDirectoryIcons);
}

void MFPMainWindow::onDoubleClicked(const QModelIndex index) {
	emit play(index.row());
}
