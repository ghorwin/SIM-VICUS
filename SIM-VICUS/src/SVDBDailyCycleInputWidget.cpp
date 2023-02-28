/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVDBDailyCycleInputWidget.h"

#include <QLocale>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QTableWidget>
#include <QHeaderView>
#include <QFontMetrics>
#include <QScrollBar>

#include <cmath>

#include <QtExt_Locale.h>


const int MARGIN_LEFT = 50;
const int MARGIN_TOP = 10;
static int MARGIN_RIGHT = 20;
static int MARGIN_BOTTOM = 25;

QRectF SVDBDailyCycleInputWidget::m_clippingRect;

SVDBDailyCycleInputWidget::SVDBDailyCycleInputWidget(QWidget *parent)
	: QWidget(parent)
{
	// create the widget content
	QHBoxLayout * hlay = new QHBoxLayout;
	m_view = new QGraphicsView(this);
	m_view->setMinimumSize(350,130);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	hlay->addWidget(m_view);
	setLayout(hlay);
	hlay->setMargin(0);

	// set some default values
	m_values = std::vector<double>(24,18);
	m_minValLimit = m_minVal = 0;
	m_maxValLimit = m_maxVal = 1;
	m_ignoreNodeMovement = true;

	QFontMetrics fm(font());
	int minWidth = fm.boundingRect("100:00").width();
#if defined(Q_OS_MAC)
	// fix width of table widget
	minWidth = std::max(46, minWidth);
#else
	// fix width of table widget
	minWidth = std::max(40, minWidth);
#endif

	int largeTextHeight = (int)(fm.lineSpacing()*1.2);
	MARGIN_BOTTOM = std::max(MARGIN_BOTTOM, 2*largeTextHeight);
	MARGIN_RIGHT = std::max(MARGIN_RIGHT, 15+fm.boundingRect("24:00").width()/2);

	// now populate the chart
	m_scene = new QGraphicsScene(this);
	m_view->setScene(m_scene);
	m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop); // fix the anchoring point to the top left
	//m_view->setRenderHints(QPainter::Antialiasing);

	QRectF frameRect = QRectF(MARGIN_LEFT, MARGIN_TOP,
		m_view->width() - MARGIN_RIGHT - MARGIN_LEFT,
		m_view->height() - MARGIN_TOP - MARGIN_BOTTOM);

	m_frameRect = m_scene->addRect(frameRect, QPen(Qt::black));
	m_frameRect->setZValue(0);
	m_ignoreNodeMovement = false;
}


void SVDBDailyCycleInputWidget::setup(const QString&, double minVal, double maxVal) {
	Q_ASSERT(minVal < maxVal);

	m_minValLimit = m_minVal = minVal;
	m_maxValLimit = m_maxVal = maxVal;

	resizeEvent(nullptr);
}


void SVDBDailyCycleInputWidget::setMinimum(double minVal) {
	if (minVal == m_minValLimit)
		return;

	Q_ASSERT(minVal < m_maxValLimit);

	m_minValLimit = m_minVal = minVal;

	rescaleMaxMinValues(); // this will update the scale based on
	resizeEvent(nullptr);
}


void SVDBDailyCycleInputWidget::setMaximum(double maxVal) {
	if (maxVal == m_maxValLimit)
		return;

	Q_ASSERT(m_minValLimit < maxVal);

	m_maxValLimit = m_maxVal = maxVal;

	rescaleMaxMinValues(); // this will update the scale based on
	resizeEvent(nullptr);
}


void SVDBDailyCycleInputWidget::setValues(const std::vector<double> & values) {
	Q_ASSERT(values.size() == 24);

	// copy initial values over the local values
	m_values = values;

	rescaleMaxMinValues(); // this will update the scale based on

	// update node positions
	updateCoordinates();
}


const std::vector<double> & SVDBDailyCycleInputWidget::values() const {
	return m_values;
}

// slots

void SVDBDailyCycleInputWidget::nodeMoved(const SVDBDailyCycleInputNode & n, const QPointF& newPos) {
	if (m_ignoreNodeMovement) return; // if we are editing the table, do not react on node movements
	// find the node number
	int i = n.m_nodeNumber;
	Q_ASSERT(i >= 0);
	Q_ASSERT(i < 24);
	double deltaVal = m_maxVal - m_minVal;
	int chart_height = m_view->height() - MARGIN_TOP - MARGIN_BOTTOM;
	double val = (chart_height + MARGIN_TOP - newPos.y())/chart_height * deltaVal + m_minVal;
	// round to a 2 digits
	val = floor(val*100 + 0.5)/100.0;
	if (val > m_maxVal)
		val = m_maxVal;
	if (val < m_minVal)
		val = m_minVal;

	if (val != m_values[i])
		emit valueChanged();
	m_values[i] = val;
	double hy = MARGIN_TOP + chart_height - (m_values[i] - m_minVal)/deltaVal * chart_height;
	int chart_width = m_view->width() - MARGIN_RIGHT - MARGIN_LEFT;
	double column_width = chart_width/24.0;
	m_barShadows[i]->setRect( QRectF(0, hy, column_width, chart_height + MARGIN_TOP - hy) );
	m_barShadowLines[i]->setLine( QLineF(0, hy, column_width, hy));
}


void SVDBDailyCycleInputWidget::updateValue(int i, double val) {
	// find index
	Q_ASSERT(i >= 0);
	Q_ASSERT(i < 24);
	// check if we have a modification
	if (m_values[i] != val) {
		m_values[i] = val;
		// rescale maximum
		rescaleMaxMinValues();
		if (!m_nodes.isEmpty()) {
			// calculate y-position of node
			double deltaVal = m_maxVal - m_minVal;
			int chart_height = m_view->height() - MARGIN_TOP - MARGIN_BOTTOM;
			// y coordinate in screen coordinates from bottom of chart
			double hy = MARGIN_TOP + chart_height - (m_values[i] - m_minVal)/deltaVal * chart_height;
			// we need to disable the notification signal for the next call to avoid cyclic repetition
			m_ignoreNodeMovement = true;
			m_nodes[i]->setPos(m_nodes[i]->x(), hy);
			int chart_width = m_view->width() - MARGIN_RIGHT - MARGIN_LEFT;
			double column_width = chart_width/24.0;
			m_barShadows[i]->setRect( QRectF(0, hy, column_width, chart_height + MARGIN_TOP - hy) );
			m_barShadowLines[i]->setLine( QLineF(0, hy, column_width, hy));
			m_ignoreNodeMovement = false;
		}
	}
	m_view->update();
}

// protected functions / events

void SVDBDailyCycleInputWidget::resizeEvent(QResizeEvent * ) {
	//const char* FUNC_ID = "SVDBDailyCycleInputWidget::resizeEvent()";

	// update positions of nodes
	//	IBK::IBK_Message( IBK::FormatString("Width '%1'.\n").arg( m_view->width()),
	//		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	int chart_width = m_view->width() - MARGIN_RIGHT - MARGIN_LEFT;
	double column_width = chart_width/24.0;
	int chart_height = m_view->height() - MARGIN_TOP - MARGIN_BOTTOM;

	m_clippingRect = QRectF(MARGIN_LEFT, MARGIN_TOP, chart_width, chart_height);
	m_frameRect->setRect(m_clippingRect);

	m_scene->setSceneRect(0, 0, m_view->width()-2, m_view->height()-2);

	QLinearGradient linearGrad(QPointF(0, 0), QPointF(column_width, 0));
	linearGrad.setColorAt(0, QColor(220,220,255));
	linearGrad.setColorAt(1, QColor(245,245,255));
	// initial setup when m_nodes is still empty
	if (m_nodes.isEmpty()) {
		for (int i=0; i<24; ++i) {
			double x = MARGIN_LEFT + column_width * i;
			QGraphicsRectItem * r = m_scene->addRect(
				QRectF(0, 0, column_width, chart_height),
				QPen(Qt::black), QBrush( linearGrad) );
			r->setPos(x, MARGIN_TOP);
			r->setZValue(0);
			m_bars.append(r);
			QGraphicsRectItem * r2 = m_scene->addRect(
				QRectF(0, 0, column_width, 0),
				QPen(Qt::NoPen), QBrush( QColor(125,125,125, 60) ) );
			m_barShadows.append(r2);
			r2->setZValue(1);
			QGraphicsLineItem * shadowLine = m_scene->addLine( 0, 0, column_width, 0, QPen(Qt::black));
			m_barShadowLines.append(shadowLine);
			shadowLine->setZValue(2);

			if (i==0) {
				SVDBDailyCycleInputEdge * edge = new SVDBDailyCycleInputEdge(x - column_width/2.0, MARGIN_TOP, x + column_width/2.0, MARGIN_TOP);
				edge->setZValue(1);
				m_scene->addItem(edge);
				m_edges.append(edge);

				QString label(tr(" 0:00"));
				QGraphicsTextItem * textItem = m_scene->addText(label);
				textItem->setZValue(3);
				m_xLabels.append(textItem);
			}
			SVDBDailyCycleInputEdge * edge = new SVDBDailyCycleInputEdge(x + column_width/2.0, MARGIN_TOP, x + 1.5*column_width, MARGIN_TOP);
			edge->setZValue(1);
			m_scene->addItem(edge);
			m_edges.append(edge);

			QString label = tr("%1:00").arg((i+1),2);
			QGraphicsTextItem * textItem = m_scene->addText(label);
			textItem->setZValue(3);
			m_xLabels.append(textItem);

			SVDBDailyCycleInputNode * n = new SVDBDailyCycleInputNode(m_edges[i], m_edges[i+1]);
			// calculate y-position of node
			double deltaVal = m_maxVal - m_minVal;
			double val = m_values[i];
			if (val > m_maxVal)	val = m_maxVal;
			if (val < m_minVal)	val = m_minVal;
			// y coordinate in screen coordinates
			double hy = MARGIN_TOP + chart_height - (val - m_minVal)/deltaVal * chart_height;
			m_barShadows[i]->setRect( QRectF(0, hy, column_width, chart_height + MARGIN_TOP - hy) );
			m_barShadowLines[i]->setLine( QLineF(0, hy, column_width, hy));

			n->setPos(x + column_width/2, hy);
			n->m_nodeNumber = i;
			m_scene->addItem(n);
			m_nodes.append(n);
			n->m_allowFreeMoving = false;
		}
		m_edges[0]->m_boundary = true;
		m_edges[24]->m_boundary = true;
		m_nodes[0]->m_otherEdge = m_edges[24];
		m_nodes[23]->m_otherEdge = m_edges[0];

		// also create the vertical labels
		QGraphicsTextItem * textItem = m_scene->addText(tr("%L1").arg(m_maxVal));
//		textItem->setPos(MARGIN_LEFT - textItem->boundingRect().width() - 2, MARGIN_TOP - textItem->boundingRect().height()/2.0);
		textItem->setZValue(3);
		m_yLabels.append(textItem);
		textItem = m_scene->addText(tr("%L1").arg(m_minVal));
//		textItem->setPos(MARGIN_LEFT - textItem->boundingRect().width() - 2, MARGIN_TOP + chart_height - textItem->boundingRect().height()/2.0);
		textItem->setZValue(3);
		m_yLabels.append(textItem);

		for (int i=0; i<24; ++i) {
			m_nodes[i]->adjustEdges(m_nodes[i]->pos());
		}
	} // end initial setup

	// adjust labels
	m_yLabels[0]->setPlainText( QString("%L1").arg(m_maxVal));
	m_yLabels[0]->setPos(MARGIN_LEFT - m_yLabels[0]->boundingRect().width() - 2,
						 MARGIN_TOP - m_yLabels[0]->boundingRect().height()/2.0);
	m_yLabels[1]->setPlainText( QString("%L1").arg(m_minVal));
	m_yLabels[1]->setPos(MARGIN_LEFT - m_yLabels[1]->boundingRect().width() - 2,
						 MARGIN_TOP + chart_height - m_yLabels[1]->boundingRect().height()/2.0);
	// only turn on every 4th label if column widths are small
	for (int i=0; i<24; ++i) {
		if (column_width < 40)
			m_xLabels[i]->setVisible(i % 4 == 0);
		else
			m_xLabels[i]->setVisible(true);
	}
	// update the coordinates
	updateCoordinates();
}


void SVDBDailyCycleInputWidget::updateCoordinates() {
	int chart_width = m_view->width() - MARGIN_RIGHT - MARGIN_LEFT;
	double column_width = chart_width/24.0;
	int chart_height = m_view->height() - MARGIN_TOP - MARGIN_BOTTOM;
	for (int i=0; i<24; ++i) {
		double x = MARGIN_LEFT + column_width * i;
		m_bars[i]->setRect( QRectF(0, 0, column_width, chart_height) );
		m_bars[i]->setPos(x, MARGIN_TOP);

		m_nodes[i]->m_allowFreeMoving = true;
		// calculate y-position of node
		double deltaVal = m_maxVal - m_minVal;
		double val = m_values[i];
		if (val > m_maxVal)	val = m_maxVal;
		if (val < m_minVal)	val = m_minVal;
		// y coordinate in screen coordinates
		double hy = MARGIN_TOP + chart_height - (val - m_minVal)/deltaVal * chart_height;

		m_ignoreNodeMovement = true;
		m_nodes[i]->setPos(x + column_width/2.0, hy); //m_nodes[i]->y()); // keep same height
		m_ignoreNodeMovement = false;
		m_nodes[i]->m_allowFreeMoving = false;

		m_barShadows[i]->setRect( QRectF(0, hy, column_width, chart_height + MARGIN_TOP - hy) );
		m_barShadows[i]->setPos(x, 0);
		m_barShadowLines[i]->setLine( QLineF(0, hy, column_width, hy));
		m_barShadowLines[i]->setPos(x, 0);

		m_xLabels[i]->setPos(x - m_xLabels[i]->boundingRect().width()/2.0, MARGIN_TOP + chart_height + 3);
	}
	m_xLabels[24]->setPos(MARGIN_LEFT + column_width * 24 - m_xLabels[24]->boundingRect().width()/2.0, MARGIN_TOP + chart_height + 3);

	QLineF l = m_edges[0]->line();
	l.setP1( QPointF(MARGIN_LEFT - column_width/2, l.p1().y()) );
	m_edges[0]->setLine(l);
	l = m_edges[24]->line();
	l.setP2( QPointF(MARGIN_LEFT + 24.5*column_width, l.p2().y()) );
	m_edges[24]->setLine(l);
}


void SVDBDailyCycleInputWidget::rescaleMaxMinValues() {
	if (m_values.empty())
		return;

	// compute maximum value of all entered values
	double maxVal = m_values[0];
	double minVal = m_values[0];

	for (unsigned int i=1; i<m_values.size(); ++i) {
		maxVal = std::max(maxVal, m_values[i]);
		minVal = std::min(minVal, m_values[i]);
	}

	// compute nice maxVal and minVals
	unsigned int NUM_TICKS = 10;
	double step = std::max(1.0, (maxVal - minVal)) / NUM_TICKS;
	double rounded_step = std::pow(10.0, std::floor(std::log10(step)));
	if (5*rounded_step < step) {
		rounded_step *= 5;
	}
	else if (2*rounded_step < step) {
		rounded_step *= 2;
	}
	minVal = std::floor(minVal/rounded_step) * rounded_step;
	maxVal = std::ceil(maxVal/rounded_step) * rounded_step;

	// obey limits
	minVal = std::min(minVal, m_minValLimit);
	maxVal = std::max(maxVal, m_maxValLimit);

	if (minVal != m_minVal || maxVal != m_maxVal) {
		m_minVal = minVal;
		m_maxVal = maxVal;
	}
	resizeEvent(nullptr);
}


// *** SVDBDailyCycleInputNode ***



SVDBDailyCycleInputNode::SVDBDailyCycleInputNode(QGraphicsLineItem * leftEdge, QGraphicsLineItem * rightEdge)
	: m_leftEdge(leftEdge), m_rightEdge(rightEdge)
{
	m_allowFreeMoving = true;
	m_hoveredOver = false;
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
	setCacheMode(DeviceCoordinateCache);
	setZValue(5);
	setAcceptHoverEvents(true);
	m_otherEdge = nullptr;
	m_nodeNumber = -1;
}


QRectF SVDBDailyCycleInputNode::boundingRect() const {
	qreal adjust = 2;
	return QRectF(-6 - adjust, -6 - adjust, 14 + adjust, 14 + adjust);
}


void SVDBDailyCycleInputNode::paint ( QPainter * painter, const QStyleOptionGraphicsItem * , QWidget * ) {
	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::darkGray);
	painter->drawEllipse(-6 +2, -6 +2, 12, 12);

	QRadialGradient gradient(-2, -2, 6);
	if (m_hoveredOver) {
		//gradient.setCenter(0, 0);
		gradient.setFocalPoint(0, 0);
		gradient.setColorAt(0, QColor(Qt::red).lighter(160));
		gradient.setColorAt(1, QColor(Qt::darkRed).lighter(160));
	}
	else {
		gradient.setColorAt(0, QColor(Qt::red));
		gradient.setColorAt(1, QColor(Qt::darkRed));
	}
	painter->setPen(QPen(Qt::black, 0));
	painter->setBrush(gradient);
	painter->drawEllipse(-6, -6, 12, 12);
}


void SVDBDailyCycleInputNode::hoverEnterEvent(QGraphicsSceneHoverEvent * event ) {
	m_hoveredOver = true;
	update();
	QGraphicsItem::hoverEnterEvent(event);
}


void SVDBDailyCycleInputNode::hoverLeaveEvent(QGraphicsSceneHoverEvent * event ) {
	m_hoveredOver = false;
	update();
	QGraphicsItem::hoverEnterEvent(event);
}


QVariant SVDBDailyCycleInputNode::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (change == ItemPositionChange && scene()) {
		// value is the new position.
		QPointF newPos = value.toPointF();
		if (m_allowFreeMoving) {
			adjustEdges(newPos);
			return QGraphicsItem::itemChange(change, value);
		}

		// oldpos is the old position.

		QPointF oldPos = pos();
		if (oldPos != newPos) {

			// fix x coordinate for this movement
			newPos.setX(oldPos.x());

			// fix y if we move to far off the widget
			if (newPos.y() > SVDBDailyCycleInputWidget::m_clippingRect.bottom())
				newPos.setY( SVDBDailyCycleInputWidget::m_clippingRect.bottom() );
			if (newPos.y() < SVDBDailyCycleInputWidget::m_clippingRect.top())
				newPos.setY( SVDBDailyCycleInputWidget::m_clippingRect.top() );

			// notify others
			adjustEdges(newPos);

			// notify parents
			if (!scene()->views().isEmpty()) {
				QGraphicsView * v = scene()->views()[0];
				SVDBDailyCycleInputWidget* widget = dynamic_cast<SVDBDailyCycleInputWidget*>(v->parent());
				widget->nodeMoved(*this, newPos);
			}

			// return new position
			return newPos;
		}
	}
	return QGraphicsItem::itemChange(change, value);
}


void SVDBDailyCycleInputNode::adjustEdges(const QPointF & newPos) {
	// update left edge
	if (m_leftEdge != nullptr) {
		QLineF l = m_leftEdge->line();
		l.setP2(newPos);
		m_leftEdge->setLine( l );
	}
	if (m_rightEdge != 0) {
		QLineF l = m_rightEdge->line();
		l.setP1(newPos);
		m_rightEdge->setLine( l );
	}

	// what is a other edge
	if (m_otherEdge != 0) {
		QLineF l = m_otherEdge->line();
		if (m_nodeNumber == 0)
			l.setP2( QPointF(l.p2().x(), newPos.y()) );	// right side
		else
			l.setP1( QPointF(l.p1().x(), newPos.y()) );	// right side
		m_otherEdge->setLine( l );
	}
}




// *** SVDBDailyCycleInputEdge ***


SVDBDailyCycleInputEdge::SVDBDailyCycleInputEdge(qreal x1, qreal y1, qreal x2, qreal)
	: QGraphicsLineItem(x1, y1, x2, y1)
{
	m_boundary = false;
}


void SVDBDailyCycleInputEdge::paint(QPainter *, const QStyleOptionGraphicsItem * , QWidget * ) {
#if 0
	if (m_boundary) {
		// set clipping rectangle
		painter->setClipping(true);
		painter->setClipRect(DailyCycleInputWidget::m_clippingRect);
	}
	// paint the bar underneath the line
/*	QLinearGradient linearGrad(QPointF(0, m_clippingRect.top()), QPointF(0, m_clippingRect.bottom()));
	linearGrad.setColorAt(0, QColor(250,0,0, 100));
	linearGrad.setColorAt(1, QColor(0,0,0, 100));
	painter->setBrush(linearGrad);
*/
	painter->setBrush( QColor(125,125,125, 60) );
	painter->setPen(Qt::NoPen);
	QPointF points[4];
	points[0] = line().p1();
	points[1] = line().p2();
	points[2] = QPointF(line().p2().x(), DailyCycleInputWidget::m_clippingRect.bottom());
	points[3] = QPointF(line().p1().x(), DailyCycleInputWidget::m_clippingRect.bottom());
	painter->drawConvexPolygon(points, 4);

	QGraphicsLineItem::paint(painter, style, w);
#endif
}
