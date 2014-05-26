#ifndef GRIDSEARCHER_H
#define GRIDSEARCHER_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QSet>
#include <QPoint>
#include <memory>
#include <queue>
#include <functional>

#include "hexgrid/gridsearchevent.h"
#include "utils/channel.h"

class HexGrid;


class GridSearcher
{
public:
	explicit GridSearcher(HexGrid &grid);
	void search(std::shared_ptr<Channel<GridSearchEvent>> outputChannel);

private:

	HexGrid &grid;
};

#endif // GRIDSEARCHER_H
