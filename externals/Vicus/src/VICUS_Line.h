#ifndef LINE_H
#define LINE_H

#include "VICUS_Edge.h"

namespace VICUS {


class Line{
public:

	Line(const double &x1, const double &y1, const double &x2, const double &y2):
		m_x1(x1),
		m_y1(y1),
		m_x2(x2),
		m_y2(y2)
	{}

	Line(const Edge &e):
		m_x1(e.m_node1->m_x),
		m_y1(e.m_node1->m_y),
		m_x2(e.m_node2->m_x),
		m_y2(e.m_node2->m_y)
	{}

	/*! return intersection point between two lines */
	void intersection(const Line &line, double &xs, double &ys) const;

	/*! return othogonal projection of point on line */
	void projectionFromPoint(const double &xp, const double &yp, double &xproj, double &yproj) const;

	/*! return orthogonal distance between point and line */
	double distanceToPoint(const double &xp, const double &yp) const;

	/*! determines wether the given point is on the line, between the determining points but does not match any of the determining points */
	bool containsPoint(const double & xp, const double &yp) const;

	/*! determine wether line shares an intersection point wiht given line. The intersection point must be within both lines */
	bool sharesIntersection(const Line &line) const;

	/*! returns length of the line */
	double length() const;

	/*! retruns distance between two given points */
	static double distanceBetweenPoints(const double &x1, const double &y1, const double &x2, const double &y2);

	/*! checks wether the distance between two points is below the threshold */
	static bool pointsMatch(const double &x1, const double &y1, const double &x2, const double &y2, const double threshold=0.01);

	double m_x1;
	double m_y1;
	double m_x2;
	double m_y2;

};

} // namespace VICUS

#endif // LINE_H
