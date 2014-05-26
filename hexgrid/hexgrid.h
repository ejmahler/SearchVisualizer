#ifndef HEXGRID_H
#define HEXGRID_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QPoint>
#include <memory>

//return -1 if val is negative, 1 if val is positive
template <typename T> inline
int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

struct GridEntry {
	enum EntryType { Start, End, Wall, Open } type;
	bool searched;
	bool queued;
	bool modified;
	bool path;

	GridEntry(void)
        :type(GridEntry::Open), searched(false), queued(false), modified(true), path(false)
	{}
};

class HexGrid : public QObject
{
	Q_OBJECT
public:
	//creates a "square" hex grid with "height" rows and "width" cells per row
	explicit HexGrid(QObject *parent, int width, int height);

	QVector<QPoint> getNeighbors(const QPoint &p) const;

	bool isValidCell(const QPoint &p) const;

	//undefined if p is not a valid cell
	GridEntry& getEntry(const QPoint &p);


	QList<QPoint> getCells(void);

	int manhattanDistance(const QPoint &p1, const QPoint &p2) const;
	int getWidth(void) const;
	int getHeight(void) const;

	void resetSearched(void);
	void resetAll(void);

private:
	QHash<QPoint,GridEntry> grid;
	int width, height;

	QVector<QPoint> neighborSet;
};

uint qHash(const QPoint &p);

#endif // HEXGRID_H
