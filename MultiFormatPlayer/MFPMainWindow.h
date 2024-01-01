#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MFPMainWindow.h"
#include "MFPluginBase.h"
class MFPMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MFPMainWindow(QWidget *parent = nullptr);
    ~MFPMainWindow();

private:
    Ui::MFPMainWindowClass ui;
    MFPluginBase* mFPluginBase;
    bool loadPlugin();
private slots:
    void onButtonClick();
};
