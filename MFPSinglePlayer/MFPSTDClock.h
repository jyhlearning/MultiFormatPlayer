#pragma once
#include "QMutex"
#include "QTime"
class MFPSTDClock
{
private:
	QTime time;
public:
	QMutex lock;
	MFPSTDClock();
	void setTime(const QTime &time);
	QTime getTime() const;
};

