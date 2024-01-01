#pragma once

#include "mfpsingleplayer_global.h"
#include "MFPluginBase.h"
#include "MFPlayerWidget.h"
#include "MFPlayerDecodeThread.h"
#include "MFPFrameQueue.h"
#include "MFPlayerThread.h"
#include "opencv2/highgui/highgui.hpp"

class MFPSINGLEPLAYER_EXPORT MFPSinglePlayer:public QObject,public MFPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MFPluginBase_IID)
    Q_INTERFACES(MFPluginBase)

private:
    MFPlayerWidget* mFPlayerWidget;
    MFPlayerDecodeThread* mFPlayerDecodeThread;
    MFPlayerThread* mFPlayerThread;
    statement state;
    MFPFrameQueue<AVFrame> *frameQueue;
    void stopThread();
public:
    MFPSinglePlayer();
    ~MFPSinglePlayer();
    void show() override;
public slots:
    void onPlay();
    void onStateChange(statement state);
private slots:
    void destroyThread();

signals:
    void startThread();
    void flagChange(bool state);
};
