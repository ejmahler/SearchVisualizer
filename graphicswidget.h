#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include <memory>
#include <QGLWidget>
#include <QStaticText>
#include <QImage>

class HexGrid;
class GridEntry;

class GraphicsWidget : public QGLWidget
{
	Q_OBJECT
public:
	explicit GraphicsWidget(QWidget *parent = 0);

	void draw(HexGrid *grid);

	QPoint pickCell(const QPointF &pos) const;

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);

	void drawDiagnosticText(QPainter &painter, int top,
		const QString &labelText, const QString &valueText);
	void drawControlText(QPainter &painter, int top,
		const QString &labelText, const QString &valueText);

	QTransform getCurrentTransform(void) const;

private:
	QMap<QString, QStaticText> staticText;

	QImage elements;

	HexGrid *grid;

	bool displayControls;

    const static QPolygonF DISPLAY_HEXAGON;

};

#endif // GRAPHICSWIDGET_H
