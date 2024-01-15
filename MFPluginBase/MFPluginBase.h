#pragma once

#include "mfpluginbase_global.h"
#include "QtPlugin"
#include "QObject"
class MFPLUGINBASE_EXPORT MFPluginBase:public QObject
{
    Q_OBJECT
public:
    MFPluginBase();
    virtual ~MFPluginBase() = default;
    virtual void show() = 0;//3
    virtual void setParent(QWidget* parent) = 0;
    virtual QWidget* getInstance() = 0;
    virtual void init(const QString& url) = 0;//2
    virtual void read(QJsonObject& obj) = 0;//1
};

QT_BEGIN_NAMESPACE
#define MFPluginBase_IID "MFP.Plugins.MFPluginBase"
Q_DECLARE_INTERFACE(MFPluginBase, MFPluginBase_IID)
QT_END_NAMESPACE
