#pragma once

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
    void loadHistory(QJsonArray history);
private:
    Ui::MFPMainWindowClass ui;
private slots:
    void onDoubleClicked(const QModelIndex index);
    void onOpenFileButton();
};
