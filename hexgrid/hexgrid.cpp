#include "hexgrid.h"


HexGrid::HexGrid(QObject *parent, int width, int height)
	:QObject(parent), width(width), height(height), neighborSet()
{
	neighborSet.reserve(6);
	neighborSet.append(QPoint(0, 1));
	neighborSet.append(QPoint(1, 0));
	neighborSet.append(QPoint(1, 1));
	neighborSet.append(QPoint(0, -1));
	neighborSet.append(QPoint(-1, 0));
	neighborSet.append(QPoint(-1, -1));

	for (int i = 0; i < height; i++)
	{
		//the y axis is actually at a 60 degree angle to the x axis rather than going up and down
		//so as we move further away from the x axis, the leftmost column that we keep track of on
		//this square-like grid will increase, by one column for every 2 rows
		int leftCol = i / 2;

		for (int j = leftCol; j < leftCol + width; j++)
		{
			grid.insert(QPoint(j, i), GridEntry());
		}
	}
}

QVector<QPoint> HexGrid::getNeighbors(const QPoint &p) const
{
	QVector<QPoint> results;
	results.reserve(neighborSet.size());

	for (const QPoint &n : neighborSet)
	{
		QPoint testPoint = p + n;
		if (grid.contains(testPoint))
		{
			results.push_back(testPoint);
		}
	}
	return results;
}

bool HexGrid::isValidCell(const QPoint &p) const
{
	return grid.contains(p);
}


GridEntry& HexGrid::getEntry(const QPoint &p)
{
	return grid[p];
}

QList<QPoint> HexGrid::getCells(void)
{
	return grid.keys();
}


int HexGrid::manhattanDistance(const QPoint &p1, const QPoint &p2) const
{
	//algorithm: http://stackoverflow.com/questions/5084801/distance-between-tiles-in-a-hexagonal-field

	int dx = p2.x() - p1.x();
	int dy = p2.y() - p1.y();

	if (sign(dx) == sign(dy))
	{
		return qMax(qAbs(dx), qAbs(dy));
	}
	else
	{
		return qAbs(dx) + qAbs(dy);
	}
}

int HexGrid::getWidth(void) const
{
	return width;
}
int HexGrid::getHeight(void) const
{
	return height;
}

void HexGrid::resetSearched(void)
{
	for (auto it = grid.begin(); it != grid.end(); it++)
	{
		it.value().searched = false;
		it.value().queued = false;
		it.value().path = false;
		it.value().modified = true;
	}
}

void HexGrid::resetAll(void)
{
	for (auto it = grid.begin(); it != grid.end(); it++)
	{
		it.value().searched = false;
		it.value().queued = false;
		it.value().path = false;
		it.value().modified = true;
		it.value().type = GridEntry::Open;
	}
}

uint qHash(const QPoint &p)
{
	uint hx = qHash(p.x());
	uint hy = qHash(p.y());

	return hx ^ ((hy << 16) | (hy >> 16));
}
