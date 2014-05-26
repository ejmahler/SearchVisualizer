#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtConcurrent>
#include <QLayout>
#include <QTimer>

#include "graphicswidget.h"
#include "hexgrid/gridpainter.h"
#include "hexgrid/gridsearcher.h"
#include "hexgrid/hexgrid.h"

MainWindow::MainWindow(QWidget *parent) :
		QWidget(parent),
        ui(new Ui::MainWindow()),
		graphicsWidget(new GraphicsWidget(this)),
        grid(new HexGrid(this, 50, 40)),

        searchTimer(new QTimer(this)),

        painter(new GridPainter(*grid)),
		searcher(nullptr),
		searchChannel(nullptr),

		leftMouseButton(false)
{
	ui->setupUi(this);

	layout()->addWidget(graphicsWidget);

	graphicsWidget->draw(grid);

	connect(searchTimer, &QTimer::timeout,
		this, &MainWindow::on_searchTimer_timeout);
}

MainWindow::~MainWindow()
{
}


void MainWindow::on_searchTimer_timeout(void)
{
	if (searchChannel != nullptr)
	{
		//try to get a thing from the search channel
		GridSearchEvent searchEvent;
		if (searchChannel->pop(searchEvent))
		{
			//take the info from the search event and use it to modify the grid
			if (searchEvent.eventType == GridSearchEvent::BACKTRACE)
			{
				grid->getEntry(searchEvent.point).path = true;
			}
			else if (searchEvent.eventType == GridSearchEvent::EXPAND)
			{
				grid->getEntry(searchEvent.point).searched = true;
			}
			grid->getEntry(searchEvent.point).modified = true;
		}
		else
		{
			//we didn't successfully get an item from the channel. the search must be over.
			searchTimer->stop();
			searchChannel = nullptr;
		}

		graphicsWidget->draw(grid);
	}
}



void MainWindow::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		leftMouseButton = true;

		QPoint pickedCell = graphicsWidget->pickCell(graphicsWidget->mapFromGlobal(QCursor::pos()));
		painter->start(pickedCell);

		mouseMoveEvent(event);
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		leftMouseButton = false;
	}
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
	if (leftMouseButton && !searchTimer->isActive())
	{
		QPoint pickedCell = graphicsWidget->pickCell(graphicsWidget->mapFromGlobal(QCursor::pos()));
		painter->paint(pickedCell);
	}
	graphicsWidget->draw(grid);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_W:
	case Qt::Key_S:
    case Qt::Key_G:
    case Qt::Key_O:
		painter->keyPressed((Qt::Key)event->key());
		break;

	case Qt::Key_Return:
	case Qt::Key_Enter:
		startSearch();
		break;

	case Qt::Key_Space:
		togglePauseSearch();
		break;

	case Qt::Key_Delete:
	case Qt::Key_Backspace:
		cancelSearch();
		break;


	case Qt::Key_Escape:
		cancelSearch();
		grid->resetAll();
		break;
	}
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
    case Qt::Key_W:
    case Qt::Key_S:
    case Qt::Key_G:
    case Qt::Key_O:
		painter->keyReleased((Qt::Key)event->key());
		break;
	}
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event)
}

void MainWindow::startSearch(void)
{
	grid->resetSearched();

	//create a new grid searcher
    searcher = std::unique_ptr<GridSearcher>(new GridSearcher(*grid));

	//create a new channel to put results into
	searchChannel = std::make_shared<Channel<GridSearchEvent>>(
		Channel<GridSearchEvent>::BLOCK, 20);

	//start the search process in a new thread
	//don't store the future object because we 100% don't care what happens to the thread
	QtConcurrent::run(
		searcher.get(),
		&GridSearcher::search,
		searchChannel
		);

	//start the timer that will pull results out every 1ms
	searchTimer->start(0);
}

void MainWindow::cancelSearch(void)
{
	if (searchTimer->isActive())
	{
		//we won't be using any more of the results of the search, so just close the channel
		searchChannel->closeFront();
		searchChannel = nullptr;

		//stop the search timer
		searchTimer->stop();
	}

	//wipe all the search results from the grid
	grid->resetSearched();

	//redraw the grid
	graphicsWidget->draw(grid);
}

void MainWindow::togglePauseSearch(void)
{
	if (searchTimer->isActive())
		searchTimer->stop();
	else
		if (searchChannel != nullptr)
			searchTimer->start();
}
