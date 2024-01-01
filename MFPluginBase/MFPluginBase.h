#pragma once

#include "mfpluginbase_global.h"
#include "QtPlugin"

class MFPLUGINBASE_EXPORT MFPluginBase
{
public:
    MFPluginBase();
    virtual ~MFPluginBase() = default;
    virtual void show() = 0;
};

QT_BEGIN_NAMESPACE
#define MFPluginBase_IID "MFP.Plugins.MFPluginBase"
Q_DECLARE_INTERFACE(MFPluginBase, MFPluginBase_IID)
QT_END_NAMESPACE
