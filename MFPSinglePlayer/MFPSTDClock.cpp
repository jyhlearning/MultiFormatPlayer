#include "MFPSTDClock.h"
MFPSTDClock::MFPSTDClock() {
	time = QTime::currentTime();
}
QTime MFPSTDClock::getTime() const {
	return time;
}
void MFPSTDClock::setTime(const QTime& time) {
	this->time = time;
}


