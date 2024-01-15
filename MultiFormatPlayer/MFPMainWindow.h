#pragma once

#include <qstandarditemmodel.h>
#include <QtWidgets/QMainWindow>
#include "ui_MFPMainWindow.h"
#include "MFPluginBase.h"
#include "QJsonArray"
class MFPMainWindow : public QMainWindow
{
    Q_OBJECT
signals:
    void play(int index);
public:
    MFPMainWindow(QWidget *parent = nullptr);
    ~MFPMainWindow();
    void addPluginWidget(QWidget *widget);
    void loadHistory();
    void setHistory(QJsonArray *array);
private:
    QStandardItemModel* ItemModel;
    QJsonArray* history;
    QAction* actionDelete,*actionClear;
    QMenu* menu;
    Ui::MFPMainWindowClass ui;
private slots:
    void onDoubleClicked(const QModelIndex index);
    void onOpenFileButton();
    void onActionDelete();
    void onActionClear();
    void onListViewRightClickRequest(const QPoint& p);
};
