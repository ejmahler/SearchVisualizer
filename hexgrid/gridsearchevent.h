#ifndef GRIDSEARCHEVENT_H
#define GRIDSEARCHEVENT_H

#include <QPoint>

struct GridSearchEvent
{
	enum EventType { NEIGHBOR, EXPAND, BACKTRACE } eventType;
	QPoint point;

	GridSearchEvent(const EventType &eventType, const QPoint &p)
		:eventType(eventType), point(p) {}
	GridSearchEvent(void) {}
};

#endif // GRIDSEARCHEVENT_H
