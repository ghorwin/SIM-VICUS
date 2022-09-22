#ifndef QtExt_GraphicsTextItemInRectH
#define QtExt_GraphicsTextItemInRectH

#include <QGraphicsTextItem>
#include <QObject>
#include <QPen>

namespace QtExt {

/*! Classe paint a text in a surrounding rect.*/
class GraphicsTextItemInRect : public QGraphicsTextItem
{
public:
	/*! Standard constructor.
		\param hatchingType Set the hatching type. HT_NoHatch draws a normal rect
		\param distance Distance between hatching elements or hatching density.
	*/
	GraphicsTextItemInRect(const QString &text, QGraphicsItem *parent = nullptr);

	/*! Paint event draws the rect and hatching if some is given.*/
	void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

	void setFillColor(const QColor& col) { m_rectFillColor = col; }

	QColor fillColor() const { return m_rectFillColor; }

	void setRectFramePen(const QPen& pen) { m_rectFramePen = pen; }

	QPen rectFramePen() const { return m_rectFramePen; }

private:
	QColor	m_rectFillColor;
	QPen	m_rectFramePen;
	qreal	m_margin;
};

} // namespace QtExt

#endif // QtExt_GraphicsTextItemInRectH
