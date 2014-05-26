#include "gridpainter.h"

GridPainter::GridPainter(HexGrid &grid) :
	grid(grid), paintMode(GridEntry::Open)
{
}

void GridPainter::start(const QPoint &cursorPos)
{
	if (grid.isValidCell(cursorPos))
	{
		const GridEntry& currentEntry = grid.getEntry(cursorPos);

		//begin drawing. the color we paint during this drawing session depends on the color
		//of the cell under the cursor. if it is the same as the color the user is trying to draw
		//switch to "erase mode" instead
		GridEntry::EntryType type = currentEntry.type;
		if (pressedKeys.contains(Qt::Key_S) && type != GridEntry::Start)
			paintMode = GridEntry::Start;

        else if (pressedKeys.contains(Qt::Key_G) && type != GridEntry::End)
			paintMode = GridEntry::End;

		else if (pressedKeys.contains(Qt::Key_W) && type != GridEntry::Wall)
			paintMode = GridEntry::Wall;

		else
			paintMode = GridEntry::Open;
	}
}

void GridPainter::keyPressed(Qt::Key key)
{
	pressedKeys.insert(key);
}
void GridPainter::keyReleased(Qt::Key key)
{
	pressedKeys.remove(key);
}

void GridPainter::paint(const QPoint &cursorPos)
{
	if (grid.isValidCell(cursorPos))
	{
		GridEntry& currentEntry = grid.getEntry(cursorPos);

		if (currentEntry.type != paintMode)
		{
			currentEntry.type = paintMode;
			currentEntry.modified = true;
		}
	}
}
