#pragma once
#include <QSlider>
#include <QObject>
class MFPControlSilder:public QSlider
{
public:
	MFPControlSilder(QWidget* parent=nullptr);
	~MFPControlSilder();
	void mousePressEvent(QMouseEvent* event) override;
};

