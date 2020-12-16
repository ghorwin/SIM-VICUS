#ifndef DELAUWRAPPER_H
#define DELAUWRAPPER_H

#include <QPolygonF>


class DelauWrapper
{
public:


	/*! Set up all points from a polygon for triangulation. */
	void setPoints(const QPolygonF &poly);

	/*! Proceed triangulation and returns tri in a vector of QPolygonF.
		Attention setPoints must run before.
	*/
	void getTriangles(std::vector<QPolygonF> & tris);

	/*! Proceed triangulation and returns the indices of triangles in a vector of size_t.
		Attention setPoints must run before.
	*/
	void getTriangleIdx(std::vector<size_t> &triIdx);

private:
	std::vector<double>			m_coords;




};

#endif // DELAUWRAPPER_H
