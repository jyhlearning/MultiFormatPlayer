#pragma once

#include <qstandarditemmodel.h>
#include <QtWidgets/QMainWindow>
#include "ui_MFPMainWindow.h"
#include "MFPluginBase.h"
#include "QJsonArray"
#include "ui_settingsDialog.h"

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
    void read(QJsonObject &obj);
    void setHistory(QJsonArray *array);
    void setFilter(const QJsonObject& obj);
    void setSettings(const QJsonObject& obj);
    void setLanguage(const QJsonObject& obj);
    void fullScreen();
    void window();
private:
    QStandardItemModel* ItemModel;
    QJsonArray* history;
    QJsonObject* obj;
    QString filter;
    QAction* actionDelete,*actionClear,*actionSetting;
    QDialog* settingsDialog;
    QMenu* menu;
    QStringList languages;
    Ui::MFPMainWindowClass ui;
    Ui::settingsDialog settingsUi;
    void loadStyleSheet(const QString fileName);
    static void hideLayout(const QLayout* layout);
    static void showLayout(const QLayout* layout);
private slots:
    void onDoubleClicked(const QModelIndex index);
    void onOpenFileButton();
    void onOpenDirButton();
    void onActionDelete();
    void onActionClear();
    void onActionSetting();
    void onListViewRightClickRequest(const QPoint& p);
    void onSettingsChanged();
};
