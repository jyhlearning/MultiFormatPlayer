#include "MFPControlSilder.h"

#include <qevent.h>
#include <qstyle.h>

int MFPControlSilder::round(double number) { return (number > 0.0) ? (number + 0.5) : (number - 0.5); }

MFPControlSilder::MFPControlSilder(QWidget* parent): QSlider(parent) {
}

MFPControlSilder::~MFPControlSilder() {
}

void MFPControlSilder::mousePressEvent(QMouseEvent* event) {
	const int currentX = event->pos().x();
	const double per = currentX * 1.0 / width();
	const int v = round(per * (maximum() - minimum()) + minimum());
	qDebug() << v;
	this->setValue(v);
	QSlider::mousePressEvent(event);
	emit press();
}

void MFPControlSilder::mouseReleaseEvent(QMouseEvent* event)
{
	const int currentX = event->pos().x();
	const double per = currentX * 1.0 / width();
	const int v = round(per * (maximum() - minimum()) + minimum());
	qDebug() << v;
	QSlider::mouseReleaseEvent(event);
	this->setValue(v);
	emit release();
}
