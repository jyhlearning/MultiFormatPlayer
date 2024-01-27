#include "MFPMain.h"

#include <QDir>
#include <QMessageBox>
#include <QPluginLoader>
#include <QJsonParseError>
#include <QTranslator>
#include "../MFPSinglePlayer/MFPOpenGLWidget.h"

bool MFPMain::loadPlugin() {
	QDir pluginsDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_WIN)
	if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
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
		QPluginLoader* pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
		QObject* plugin = pluginLoader->instance();
		if (plugin) {
			mFPluginBase = qobject_cast<MFPluginBase*>(plugin);
			if (mFPluginBase)
				return true;
			pluginLoader->unload();
		}
	}

	return false;
}

void MFPMain::read() {
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::about(NULL, tr("提示"), tr("请检查settings.json文件"));
		return;
	}
	const QByteArray allData = file.readAll();
	file.close();

	QJsonParseError jsonError;
	const QJsonDocument jsonDoc = QJsonDocument::fromJson(allData, &jsonError);
	if (jsonError.error != QJsonParseError::NoError) {
		QMessageBox::about(NULL, tr("提示"), tr("settings.json配置文件解析出错"));
		return;
	}
	obj = jsonDoc.object();
	history = obj.value("history").toArray();
}

void MFPMain::write() {
	obj["history"] = history;
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::about(NULL, tr("提示"), tr("请检查settings.json文件"));
		return;
	}
	file.write(QJsonDocument(obj).toJson());
	file.close();
}

MFPMain::MFPMain() {
	path = "settings.json";
	read();

	QTranslator translator1, translator2;
	if (translator1.load(obj["defaultLanguage"].toString()))
		qApp->installTranslator(&translator1);
	if (translator2.load("../MFPSinglePlayer/" + obj["defaultLanguage"].toString()))
		qApp->installTranslator(&translator2);

	w = new MFPMainWindow;
	w->setHistory(&history);
	w->loadHistory();
	w->read(obj);
	if (loadPlugin()) {
		w->addPluginWidget(mFPluginBase->getInstance());

		connect(mFPluginBase,SIGNAL(sendMessage(option)), this,SLOT(receiveMessage(option)));

		mFPluginBase->moveToThread(new QThread(this));
		mFPluginBase->thread()->start();
		mFPluginBase->show();
	}
	connect(w,SIGNAL(play(int)), this,SLOT(onPlay(int)));
	connect(w, SIGNAL(destroyed()), this, SLOT(destroyThread()));
}

MFPMain::~MFPMain() {
	write();
	delete w;
}

void MFPMain::show() { w->show(); }

void MFPMain::destroyThread() {
	if (mFPluginBase) {
		if (mFPluginBase->thread()->isRunning()) {
			mFPluginBase->thread()->quit();
			mFPluginBase->thread()->wait();
			mFPluginBase->deleteLater();
		}
	}
}

void MFPMain::receiveMessage(option o) {
	switch (o) {
	case FULLSCREEN:
		w->fullScreen();
		break;
	case WINDOW:
		w->window();
		break;
	default: ;
	}
}

void MFPMain::onPlay(int index) {
	const QString url = history[index].toString();
	QJsonObject a = obj.value("singlePlayer").toObject();
	mFPluginBase->read(a);
	mFPluginBase->init(url);
	w->addPluginWidget(mFPluginBase->getInstance());
	//temp->show();
}
