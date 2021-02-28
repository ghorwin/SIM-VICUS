#ifndef SVDBDailyCycleInputWidgetH
#define SVDBDailyCycleInputWidgetH

#include <QWidget>
#include <QList>
#include <QGraphicsItem>
#include <QGraphicsLineItem>

#include <vector>

class QTableWidget;
class QTableWidgetItem;
class QGraphicsView;
class SVDBDailyCycleInputNode;
class SVDBDailyCycleInputEdge;


/*! The widget provides a graphical representation of 24 hourly values
	where the user can move each hourly step value by moving a slider.

	Initialize the widget first by calling setup().
*/
class SVDBDailyCycleInputWidget : public QWidget {
	Q_OBJECT
public:
	SVDBDailyCycleInputWidget(QWidget *parent = nullptr);

	/*! Initializes the widget with the necessary data.
	*/
	void setup(const QString& valueCaption, double minVal, double maxVal);

	/*! Set only minimum value. Keeps all other values unchanged.
		Given value must be lower than maximum value.
	*/
	void setMinimum(double minVal);

	/*! Set only maximum value. Keeps all other values unchanged.
		Given value must be greater than minimum value.
	*/
	void setMaximum(double maxVal);

	/*! Sets new values. */
	void setValues(const std::vector<double> & values);

	/*! Returns the vector with the current data. */
	const std::vector<double> & values() const;

	/*! Return minimum value.*/
	double minimum() const { return m_minValLimit; }

	/*! Return minimum value.*/
	double maximum() const { return m_maxValLimit; }

	void updateValue(int i, double value);

	static QRectF m_clippingRect;

signals:
	void valueChanged();

protected:
	virtual void resizeEvent(QResizeEvent * event);

private:
	void updateCoordinates();
	void rescaleMaxMinValues();
	void nodeMoved(const SVDBDailyCycleInputNode & n, const QPointF& newPos);

	std::vector<double>				m_values;
	double							m_minVal;
	double							m_maxVal;
	double							m_minValLimit;
	double							m_maxValLimit;

	QGraphicsView					*m_view;

	QGraphicsScene					*m_scene;
	QGraphicsRectItem				*m_frameRect;

	QList<QGraphicsRectItem*>		m_bars;
	QList<QGraphicsRectItem*>		m_barShadows;
	QList<QGraphicsLineItem*>		m_barShadowLines;
	QList<SVDBDailyCycleInputEdge*>	m_edges;
	QList<SVDBDailyCycleInputNode*>	m_nodes;
	QList<QGraphicsTextItem*>		m_xLabels;
	QList<QGraphicsTextItem*>		m_yLabels;

	bool							m_ignoreNodeMovement;

	friend class SVDBDailyCycleInputNode;
};



/*! Class used in the DailyCycleInputWidget. */
class SVDBDailyCycleInputNode : public QGraphicsItem {
public:
	SVDBDailyCycleInputNode(QGraphicsLineItem * leftEdge, QGraphicsLineItem * rightEdge);

	virtual QRectF boundingRect() const;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void adjustEdges(const QPointF & newPos);

	bool m_allowFreeMoving;

	QGraphicsLineItem	* m_otherEdge;
	int					m_nodeNumber;

protected:
	virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value );

private:

	QGraphicsLineItem	* m_leftEdge;
	QGraphicsLineItem	* m_rightEdge;

	bool m_hoveredOver;
};



/*! Class used in the SVDBDailyCycleInputWidget. */
class SVDBDailyCycleInputEdge : public QGraphicsLineItem {
public:
	SVDBDailyCycleInputEdge(qreal x1, qreal y1, qreal x2, qreal y2);

	bool			m_boundary;

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};


#endif // SVDBDailyCycleInputWidgetH
