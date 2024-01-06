#pragma once
#include <QSlider>
#include <QObject>

class MFPControlSilder : public QSlider {
	Q_OBJECT

private:
	static int round(double number);

public:
	MFPControlSilder(QWidget* parent = nullptr);
	~MFPControlSilder();
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
signals:
	void press();
	void release();
};
