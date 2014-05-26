#ifndef GRIDPAINTER_H
#define GRIDPAINTER_H

#include <QKeyEvent>

#include "hexgrid/hexgrid.h"

class GridPainter
{
public:
	explicit GridPainter(HexGrid &grid);

	void start(const QPoint &cursorPos);
	void keyPressed(Qt::Key key);
	void keyReleased(Qt::Key key);
	void paint(const QPoint &cursorPos);

private:
	HexGrid &grid;

	QSet<Qt::Key> pressedKeys;
	GridEntry::EntryType paintMode;
};

#endif // GRIDPAINTER_H
