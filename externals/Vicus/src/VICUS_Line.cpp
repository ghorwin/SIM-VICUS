#include "VICUS_Line.h"

#include <algorithm>
#include <cmath>

namespace VICUS {

void Line::intersection(const Line &line, double &xs, double &ys) const
{
	double x1 = m_x1; double x2 = m_x2; double x3 = line.m_x1; double x4 = line.m_x2;
	double y1 = m_y1; double y2 = m_y2; double y3 = line.m_y1; double y4 = line.m_y2;
	xs = ( (x4 - x3) * (x2 * y1 - x1 * y2) - (x2 - x1) * (x4 * y3 - x3 * y4) ) / ( (y4 - y3) * (x2 - x1) - (y2 - y1) * (x4 - x3) );
	ys = ( (y1 - y2) * (x4 * y3 - x3 * y4) - (y3 - y4) * (x2 * y1 - x1 * y2) ) / ( (y4 - y3) * (x2 - x1) - (y2 - y1) * (x4 - x3) );
}


void Line::projectionFromPoint(const double &xp, const double &yp, double &xproj, double &yproj) const{
	// vector form g = a + s*b;  with a = (m_x1, m_y1)
	double b1 = m_x2 - m_x1;
	double b2 = m_y2 - m_y1;
	double s = (xp + b2/b1*yp - m_x1 - b2/b1*m_y1) / (b1 + b2*b2/b1);
	xproj = m_x1 + s*b1;
	yproj = m_y1 + s*b2;
}


double Line::distanceToPoint(const double &xp, const double &yp) const{
	double xproj, yproj;
	projectionFromPoint(xp, yp, xproj, yproj);
	if (containsPoint(xproj, yproj))
		return distanceBetweenPoints(xp, yp, xproj, yproj);
	else
		return std::min(distanceBetweenPoints(m_x1, m_y1, xp, yp), distanceBetweenPoints(m_x2, m_y2, xp, yp));
}


bool Line::containsPoint(const double &xp, const double &yp) const
{
	bool inside = (xp >= std::min(m_x1, m_x2)) && (xp <= std::max(m_x1, m_x2)) && (yp >= std::min(m_y1, m_y2)) && (yp <= std::max(m_y1, m_y2));
	bool identity = pointsMatch(xp, yp, m_x1, m_y1) || pointsMatch(xp, yp, m_x2, m_y2);
	return inside && !identity;
}

bool Line::sharesIntersection(const Line &line) const
{
	double xp, yp;
	intersection(line, xp, yp);
	return containsPoint(xp, yp) && line.containsPoint(xp, yp);
}

double Line::length() const
{
	return distanceBetweenPoints(m_x1, m_y1, m_x2, m_y2);
}

double Line::distanceBetweenPoints(const double &x1, const double &y1, const double &x2, const double &y2)
{
	/// \note Instead of using the slow std::pow(x1-x2, 2), always use (x1-x2)*(x1-x2) which is way faster!
	return std::sqrt( std::pow(x1-x2, 2) + std::pow(y1-y2, 2) );
}

bool Line::pointsMatch(const double &x1, const double &y1, const double &x2, const double &y2, const double threshold)
{
	return distanceBetweenPoints(x1, y1, x2, y2) < threshold;
}


} // namespace VICUS
