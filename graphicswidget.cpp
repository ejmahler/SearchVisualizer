#include "graphicswidget.h"

#include <cmath>

#include <QDebug>
#include <QResizeEvent>


#include "hexgrid/hexgrid.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

const QPolygonF GraphicsWidget::DISPLAY_HEXAGON = QPolygonF({
    QPointF( 0.333,    0.666),//top
	QPointF(-0.333,    0.333), //top right
	QPointF(-0.666,   -0.333), //bottom right
    QPointF(-0.333,   -0.666),//bottom
	QPointF( 0.333,   -0.333), //bottom left
	QPointF( 0.666,    0.333), //top left
    });



GraphicsWidget::GraphicsWidget(QWidget *parent) :
QGLWidget(parent), elements(size(), QImage::Format_ARGB32_Premultiplied), grid(nullptr)
{
}

void GraphicsWidget::draw(HexGrid *g)
{
	grid = g;
	update();
}




void GraphicsWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

	//set up painting with antialiasing etc
	QPainter painter;

	painter.begin(&elements);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);

	//transform coordinates to fit everything neatly on the screen
	painter.save();
	painter.setWorldTransform(getCurrentTransform());

	if (grid != nullptr)
	{
		//loop through and draw each entry
		for (const auto& cell: grid->getCells())
		{
			GridEntry &entry = grid->getEntry(cell);
			if (entry.modified) {
				entry.modified = false;
				painter.save();

				painter.translate(cell.x(), cell.y());

				QPen pen(Qt::black);
				pen.setWidthF(0);

				painter.setPen(pen);
				if (entry.type == GridEntry::Wall)
					painter.setBrush(Qt::darkBlue);

				else if (entry.type == GridEntry::Start)
					painter.setBrush(Qt::yellow);

				else if (entry.type == GridEntry::End)
					painter.setBrush(Qt::red);

				else if (entry.path)
					painter.setBrush(Qt::white);

				else if (entry.searched)
					painter.setBrush(Qt::cyan);
				
				else
					painter.setBrush(Qt::darkGreen);

                painter.drawConvexPolygon(DISPLAY_HEXAGON);

				painter.restore();
			}
		}
	}

	painter.restore();
	painter.setPen(Qt::white);

	QPainter screenPainter(this);
	screenPainter.fillRect(rect(), QBrush(Qt::black));
	screenPainter.drawImage(0, 0, elements);
}

void GraphicsWidget::resizeEvent(QResizeEvent *event) {
	elements = QImage(event->size(), QImage::Format_ARGB32_Premultiplied);
	elements.fill(Qt::transparent);

	if (grid != nullptr)
	{
		//loop through and mark each entry as modified so it gets redrawn
		for (const auto& cell : grid->getCells())
		{
			grid->getEntry(cell).modified = true;
		}
	}
}

QPoint GraphicsWidget::pickCell(const QPointF &pos) const
{
    //transform the weird skewed coordinates back to
	QPointF localPos = getCurrentTransform().inverted().map(pos);

	int roundedX = qRound(localPos.x());
	int roundedY = qRound(localPos.y());

    //subtract the square grid cell we're on from the floating point location
	float compX = localPos.x() - roundedX;
	float compY = localPos.y() - roundedY;

    //if the point is inside the hexagon then we know it would have been inside the hexagon when it was unskewed
    if (sign(compX) == sign(compY) || DISPLAY_HEXAGON.containsPoint(QPointF(compX, compY), Qt::OddEvenFill))
	{
		return QPoint(roundedX, roundedY);
	}
	else
	{
		//we can tell if the correct hexagon is above, below, left, or right purely by comparing compX and compY
		//if compX + compY is greater than zero, we know it must be y+1 or x+1
		if (compX + compY > 0) {

			//if compY is greater than compX, it is y+1
			if (compY > compX)
				return QPoint(roundedX, roundedY + 1);
			else
				return QPoint(roundedX + 1, roundedY);
		}
		else
		{
			//it must be y-1 or x-1

			//if compY is less than compX, it must be y-1
			if (compY < compX)
				return QPoint(roundedX, roundedY - 1);
			else
				return QPoint(roundedX - 1, roundedY);
		}
	}
}



QTransform GraphicsWidget::getCurrentTransform(void) const
{
	QTransform t;

	if (grid != nullptr)
	{
		//each hexagon is composed of 6 equilateral triangles. we don't explicitly draw the triangles
		//but it helps to know the side length of those triangles
		//this comes from the pythagorean theorem. if the height of an equilateral triangle is h, then
		//the side length is 2*h / sqrt(h). in this case h is 0.5, so the side length is 1 / sqrt(3)
		float triangleSide = 1 / sqrt(3);
		float lineHeight = triangleSide * 1.5;
		float lineWidth = 1;

		//the columnhs shift left/right every other row,
		//so the "actual" width is one half wider than the stated width
		float gridWidth = (grid->getWidth() + 0.5) * lineWidth;

		//the cells are regular hexagons, so the tips point above/below the actual row
		//0.5 of a unit above, and 0.5 of a unit below
		float gridHeight = (grid->getHeight() + 1) * lineHeight;

		float screenFill = 0.98;
		float screenWidth = width() * screenFill;
		float screenHeight = height() * screenFill;

		//find the ratio from screen width to grid width, and screen height to grid height
		float widthRatio = screenWidth / gridWidth;
		float heightRatio = screenHeight / gridHeight;

		float scale = qMin(widthRatio, heightRatio);



		//vertically and horizontally center the resulting data
		float resultHeight = gridHeight * scale;
		float resultWidth = gridWidth * scale;

		float translateX = (width() - resultWidth) / 2 + lineWidth*scale * 2;
		float translateY = (height() - resultHeight) / 2 + lineHeight*scale;

		//apply the transformations to the matrix
		t.shear(-triangleSide, 0);
		t.translate(translateX, translateY);
		t.scale(scale, scale * lineHeight);
	}

	return t;
}


void GraphicsWidget::drawDiagnosticText(QPainter &painter, int top,
	const QString &labelText, const QString &valueText)
{
	int labelLeft = 5;
	int labelWidth = 140;
	int valueLeft = labelLeft + labelWidth + 5;
	int valueWidth = 100;

	if (!staticText.contains(labelText))
	{
		QStaticText t(labelText);
		t.setPerformanceHint(QStaticText::AggressiveCaching);
		t.setTextFormat(Qt::PlainText);
		t.setTextWidth(labelWidth);
		t.setTextOption(QTextOption(Qt::AlignRight));

		staticText.insert(labelText, t);
	}

	painter.drawStaticText(labelLeft, top, staticText.value(labelText));
	painter.drawText(QRect(valueLeft, top, valueWidth, 20), valueText, QTextOption(Qt::AlignRight));
}

void GraphicsWidget::drawControlText(QPainter &painter, int top,
	const QString &labelText, const QString &valueText)
{
	int labelLeft = width() - 200 + 5;
	int labelWidth = 40;
	int valueLeft = labelLeft + labelWidth + 10;
	int valueWidth = 150;

	if (!staticText.contains(labelText))
	{
		QStaticText t(labelText);
		t.setPerformanceHint(QStaticText::AggressiveCaching);
		t.setTextFormat(Qt::PlainText);
		t.setTextWidth(labelWidth);
		t.setTextOption(QTextOption(Qt::AlignRight));

		staticText.insert(labelText, t);
	}

	if (!staticText.contains(valueText))
	{
		QStaticText t(valueText);
		t.setPerformanceHint(QStaticText::AggressiveCaching);
		t.setTextFormat(Qt::PlainText);
		t.setTextWidth(valueWidth);
		t.setTextOption(QTextOption(Qt::AlignLeft));

		staticText.insert(valueText, t);
	}

	painter.drawStaticText(labelLeft, top, staticText.value(labelText));
	painter.drawStaticText(valueLeft, top, staticText.value(valueText));
}
