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
	menu = new QMenu(this);
	menu->addAction(actionClear);
	menu->addAction(actionDelete);
	ui.historyListView->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
	connect(ui.historyListView,SIGNAL(doubleClicked(QModelIndex)), this,SLOT(onDoubleClicked(QModelIndex)));
	connect(ui.openfileButton, SIGNAL(clicked()), this, SLOT(onOpenFileButton()));
	connect(actionDelete, SIGNAL(triggered()), this, SLOT(onActionDelete()));
	connect(actionClear, SIGNAL(triggered()), this, SLOT(onActionClear()));
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

void MFPMainWindow::setHistory(QJsonArray* array) { history = array; }

void MFPMainWindow::setFilter(const QJsonObject& obj)
{
	QJsonArray temp = obj.value("format").toArray();
	
	for (auto s:temp) {
		filter += " *." + s.toString();
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

void MFPMainWindow::onListViewRightClickRequest(const QPoint& p) {
	menu->exec(QCursor::pos());
}

void MFPMainWindow::onDoubleClicked(const QModelIndex index) {
	emit play(index.row());
}
