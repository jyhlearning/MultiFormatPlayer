#include "MFPMainWindow.h"
#include "QMessageBox"
#include "QDir"
#include "QPluginLoader"

MFPMainWindow::MFPMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    if (!loadPlugin()) {
        QMessageBox::information(this, "Error", "Could not load the plugin");
    }
    connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(onButtonClick()));
}

MFPMainWindow::~MFPMainWindow() {
    if (mFPluginBase)
        delete mFPluginBase;
}

void MFPMainWindow::onButtonClick()
{
    mFPluginBase->show();
}
bool MFPMainWindow::loadPlugin()
{
    QDir pluginsDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    pluginsDir.cd("Plugins");
    const QStringList entries = pluginsDir.entryList(QDir::Files);
    for (const QString& fileName : entries) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject* plugin = pluginLoader.instance();
        if (plugin) {
            mFPluginBase = qobject_cast<MFPluginBase*>(plugin);
            if (mFPluginBase)
                return true;
            pluginLoader.unload();
        }
    }

    return false;
}

