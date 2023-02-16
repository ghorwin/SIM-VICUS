#include "SVNonEquidistantSlider.h"
#include "ui_SVNonEquidistantSlider.h"

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QScreen>
#include <QApplication>

#include <algorithm>
#include <limits>


SVNonEquidistantSlider::SVNonEquidistantSlider(QWidget *parent) :
	QWidget(parent),
	m_currentIndex(0),
	m_reversed(false),
	m_leftMargin(2),
	m_rightMargin(4),
	m_topMargin(2),
	m_markerHeight(5),
	m_ticHeight(5),
	m_dpiFact(1.0),
	m_minTickDist(4)
{
	setEnabled(false);
	qreal dpi = QApplication::screens().at(0)->physicalDotsPerInch();
	if( dpi > 96) {
		m_dpiFact = dpi / 96.0;
		m_markerHeight *= m_dpiFact;
		m_markerHeight *= m_dpiFact;
		m_leftMargin *= m_dpiFact;
		m_rightMargin *= m_dpiFact;
		m_topMargin *= m_dpiFact;
		m_minTickDist *= m_dpiFact;
	}
}


double SVNonEquidistantSlider::currentValue() const {
	if (m_values.empty())
		return std::numeric_limits<double>::quiet_NaN();
	Q_ASSERT(m_currentIndex < m_values.size());
	return m_values[m_currentIndex];
}


double SVNonEquidistantSlider::valueFromIndex(unsigned int index) const {
	if (m_values.empty())
		return std::numeric_limits<double>::quiet_NaN();
	Q_ASSERT(index < m_values.size());
	return m_values[index];
}


void SVNonEquidistantSlider::setValues(const double* values, unsigned int size) {
	if (size == 0) {
		m_values.clear();
		m_currentIndex = static_cast<unsigned int>(-1);
		setEnabled(false);
	}
	else {
		m_values.resize(size);
		std::copy(values, values + size, m_values.begin());
		m_reversed = m_values.front() > m_values.back();
		if (m_currentIndex >= size)
			setCurrentIndex(size-1);
		setEnabled(true);
	}
}



void SVNonEquidistantSlider::setCurrentValue(double v) {
	if (m_values.empty())
		return;
	if (v != m_values[m_currentIndex]) {
		/// \todo check if lower_bound is really what we mean here... think about rounding errors
		///       v=0.9999999 -> should be: m_value[newIndex+1] = 1.0
		///       but current code gives:   m_value[newIndex] = 0.9
		unsigned int newIndex = (unsigned int)(std::lower_bound(m_values.begin(), m_values.end(), v) - m_values.begin());
		if (newIndex == m_values.size())
			--newIndex;
		if (newIndex > 0) {
			double difference = m_values[newIndex] - v;
			if (v - m_values[newIndex - 1] < difference) {
				--newIndex;
			}
		}
		setCurrentIndex(newIndex);
	}
}


void SVNonEquidistantSlider::moveIndex(int move) {
	if (m_values.empty())
		return;

	int newIndex = m_currentIndex + move;
	if (newIndex < 0)
		newIndex = 0;
	int maxIndex = (int)m_values.size() - 1;
	if (newIndex > maxIndex)
		newIndex = maxIndex;
	setCurrentIndex(newIndex);
}


void SVNonEquidistantSlider::setCurrentIndex(unsigned int index) {
	if (m_values.empty())
		return;
	if (m_currentIndex == index)
		return; // do nothing if index hasn't changed
	m_currentIndex = index;
	emit indexChanged(m_currentIndex);
	update(); // triggers paint event
}


std::pair<unsigned int,unsigned int> SVNonEquidistantSlider::leftRight() const {
	std::pair<unsigned int,unsigned int> result;
	result.first = m_leftMargin;
	result.second = width() - m_rightMargin;
	return result;
}


std::pair<double,double> SVNonEquidistantSlider::valueLeftRight() const {
	std::pair<double,double> result;
	if (m_values.empty()) {
		result.first = 0;
		result.second = 0;
	}
	else {
		result.first = m_values.front();
		result.second = m_values.back();
	}
	return result;
}


// *** protected functions ***

void SVNonEquidistantSlider::paintEvent(QPaintEvent*e) {
	QWidget::paintEvent(e);

	QPainter painter(this);

	int lineY = m_topMargin + m_markerHeight + m_ticHeight / 2;
	std::pair<unsigned int,unsigned int> leftright = leftRight();
	painter.drawLine(leftright.first, lineY, leftright.second, lineY);

	if (m_values.empty())
		return;

	double v = m_values[0];
	int x_last = xForValue(v);
	painter.drawLine(x_last,lineY - m_ticHeight / 2, x_last, lineY + m_ticHeight / 2);

	for (unsigned int i=1; i<m_values.size(); ++i){
		double v = m_values[i];
		int x = xForValue(v);
		if( (x - x_last) >= (int)m_minTickDist) {
			painter.drawLine(x,lineY - m_ticHeight / 2, x, lineY + m_ticHeight / 2);
			x_last = x;
		}
//		QString s = QString("%1").arg(v);
		//		painter.drawText(x - 5,lineY + m_ticHeight / 2 + 10,s);
	}

	painter.translate(xForValue(m_values[m_currentIndex]),0);
	painter.setBrush(QColor("red"));
	static const QPoint marker[3] = {
		QPoint(-3 * m_dpiFact, m_topMargin),
		QPoint(3 * m_dpiFact, m_topMargin),
		QPoint(0, m_topMargin+m_markerHeight)
	};
	if (isEnabled())
		painter.drawConvexPolygon(marker,3);
}


void SVNonEquidistantSlider::mouseMoveEvent(QMouseEvent *e) {
	int x = qMax(0, e->x());
	int newIndex = indexForX(x);
	setCurrentIndex(newIndex);
}


void SVNonEquidistantSlider::mousePressEvent(QMouseEvent *e){
	int newIndex = indexForX(e->x());
	setCurrentIndex(newIndex);
}


// *** private functions ***

unsigned int SVNonEquidistantSlider::xForValue(double d) {
	Q_ASSERT(!m_values.empty());

	std::pair<double,double> valleftright = valueLeftRight();
	std::pair<int,int> leftright = leftRight();

	unsigned int length = leftright.second - leftright.first;
	double vallength = valleftright.second - valleftright.first;

	if(valleftright.first == valleftright.second) {
		return length/2 + leftright.first;
	}

	if( !m_reversed) {
		if( d <= valleftright.first)
			return leftright.first;
		if( d >= valleftright.second)
			return leftright.second;
	}
	else {
		if( d >= valleftright.first)
			return leftright.first;
		if( d <= valleftright.second)
			return leftright.second;
	}

	Q_ASSERT(vallength > 0);
	double res = leftright.second - (length / vallength * (valleftright.second - d));
	return res;
}


double SVNonEquidistantSlider::valueForX(unsigned int x) {
	Q_ASSERT(!m_values.empty());

	std::pair<unsigned int,unsigned int> leftright = leftRight();
	std::pair<double,double> valleftright = valueLeftRight();
	if( x <= leftright.first)
		return valleftright.first;
	if( x >= leftright.second)
		return valleftright.second;
	int length = leftright.second - leftright.first;
	Q_ASSERT(length > 0);
	double vallength = valleftright.second - valleftright.first;

	double res = valleftright.second - (vallength / length * (leftright.second - x));
	return res;
}


unsigned int SVNonEquidistantSlider::indexForX(unsigned int x) {
	Q_ASSERT(!m_values.empty());

	double val = valueForX(x);
	unsigned int index = nearestIndex(val);
	return index;
}


unsigned int SVNonEquidistantSlider::nearestIndex(double v) {
	Q_ASSERT(!m_values.empty());

	double distance = qAbs(v - m_values[0]);
	unsigned int foundIndex = 0;

	for (unsigned int i=1; i<m_values.size(); ++i) {
		double newDistance = qAbs(v - m_values[i]);
		if (newDistance < distance) {
			distance = newDistance;
			foundIndex = i;
		}
		else {
			break;
		}
	}

	return foundIndex;
}

