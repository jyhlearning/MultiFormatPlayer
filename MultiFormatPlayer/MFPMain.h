#pragma once
#include <qfile.h>
#include <QPluginLoader>

#include "MFPMainWindow.h"
#include "QObject"
#include "QThreadPool"
class MFPMain:public QObject
{
	Q_OBJECT
private:
	MFPMainWindow* w;
	MFPluginBase* mFPluginBase;
	QThreadPool pool;
	QString path;
	QJsonArray history;
	QJsonObject obj;

	bool loadPlugin();
	void read();
	void write();
public:
	MFPMain();
	~MFPMain();
	void show();
private slots:
	void onPlay(int index);
	void destroyThread();
	void receiveMessage(option o);
};

