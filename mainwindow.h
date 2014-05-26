#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include <memory>

#include "hexgrid/gridsearchevent.h"
#include "utils/channel.h"

class QTimer;

class GraphicsWidget;
class GridPainter;
class HexGrid;
class GridSearcher;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	void on_searchTimer_timeout(void);

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);

	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

	void wheelEvent(QWheelEvent *event);

	void startSearch(void);
	void togglePauseSearch(void);
	void cancelSearch(void);


	std::unique_ptr<Ui::MainWindow> ui;

	GraphicsWidget *graphicsWidget;
	HexGrid *grid;

	QTimer *searchTimer;

	std::unique_ptr<GridPainter> painter;
	std::unique_ptr<GridSearcher> searcher;
	std::shared_ptr<Channel<GridSearchEvent>> searchChannel;

	bool leftMouseButton;
};

#endif // MAINWINDOW_H
