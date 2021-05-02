#include "VICUS_Polygon3D.h"

#include <set>

#include <QLineF>
#include <QPolygonF>

#include <IBK_Line.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include <IBKMK_3DCalculations.h>

#include <NANDRAD_Utilities.h>


#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

static int crossProdTest(QPointF a, QPointF b, QPointF c){

	if(a.y() == b.y() && a.y() == c.y()){
		if(	(b.x()<= a.x() && a.x() <= c.x()) ||
				(c.x()<= a.x() && a.x() <= b.x()))
			return 0;
		else
			return 1;
	}

	if(b.y()> c.y()){
		QPointF temp;
		temp = c;
		c=b;
		b=temp;
	}

	if (a.y() <= b.y() || a.y() > c.y())
		return 1;

	double delta = (b.x() - a.x()) * (c.y() - a.y()) -(b.y() - a.y()) * (c.x() - a.x());
	if(delta > 0)			return	1;
	else if(delta < 0)		return	-1;
	else					return	0;
}

/*! Point in Polygon function. Result:
	-1 point not in polyline
	0 point on polyline
	1 point in polyline

	\param	point test point
	Source https://de.wikipedia.org/wiki/Punkt-in-Polygon-Test_nach_Jordan

*/
static int pointInPolygon(const QPointF &point, const QPolygonF& poly)
{
	int t=-1;
	size_t polySize = (size_t)poly.size();
	for (size_t i=0; i<polySize; ++i) {
		t *= crossProdTest(point, poly.value(i), poly.value((i+1)%polySize));//  m_polyline[(i+1)%m_polyline.size()]);
		if(t==0)
			break;
	}

	return  t;
}


// *** Polygon3D ***

Polygon3D::Polygon3D(const std::vector<IBKMK::Vector3D> & vertexes) {
	setVertexes(vertexes);
}


void Polygon3D::readXML(const TiXmlElement * element) {
	FUNCID(Polygon3D::readXML);

	// read data (throws an exception in case of errors)
	NANDRAD::readVector3D(element, "Polygon3D", m_vertexes);

	unsigned int nVert = m_vertexes.size();
	checkPolygon();
	if (nVert != m_vertexes.size())
		IBK::IBK_Message(IBK::FormatString("Invalid polygon in project, removed invalid vertexes."), IBK::MSG_WARNING, FUNC_ID);
}


TiXmlElement * Polygon3D::writeXML(TiXmlElement * parent) const {
	if (*this == Polygon3D())
		return nullptr;

	return NANDRAD::writeVector3D(parent, "Polygon3D", m_vertexes);
}


// Comparison operator !=
bool Polygon3D::operator!=(const Polygon3D &other) const {
	if (m_type != other.m_type)
		return true;
	if (m_vertexes != other.m_vertexes)
		return true;
	return false;
}


void Polygon3D::addVertex(const IBK::point2D<double> & v) {
	m_vertexes.push_back(v);
	checkPolygon(); // if we have a triangle/rectangle, this is detected here
}


void Polygon3D::removeVertex(unsigned int idx){
	FUNCID(Polygon3D::removeVertex);
	if (idx >= (unsigned int)m_vertexes.size())
		throw IBK::Exception(IBK::FormatString("Index %1 out of range (vertex count = %2).").arg(idx).arg(m_vertexes.size()), FUNC_ID);
	m_vertexes.erase(m_vertexes.begin()+idx);
	m_type = T_Polygon; // assume the worst
	checkPolygon(); // if we have a triangle/rectangle, this is detected here
}


void Polygon3D::checkPolygon() {
	m_valid = false;
	eleminateColinearPts();

	// try to simplify polygon to internal rectangle/parallelogram definition
	// this may change m_type to Rectangle or Triangle and subsequently speed up operations
	detectType();
	updateLocalCoordinateSystem();
	// we need 3 vertexes (not collinear) to continue
	if (m_vertexes.size() < 3)
		return;

	// polygon must not be winding into itself, otherwise triangulation would not be meaningful
	m_valid = isSimplePolygon();
}


void Polygon3D::flip() {
	std::vector<IBKMK::Vector3D>(m_vertexes.rbegin(), m_vertexes.rend()).swap(m_vertexes);
}


void Polygon3D::detectType() {
	if (m_vertexes.size() == 3) {
		m_type = T_Triangle;
		return;
	}
	if (m_vertexes.size() != 4)
		return;
	const IBKMK::Vector3D & a = m_vertexes[0];
	const IBKMK::Vector3D & b = m_vertexes[1];
	const IBKMK::Vector3D & c = m_vertexes[2];
	const IBKMK::Vector3D & d = m_vertexes[3];
	IBKMK::Vector3D c2 = b + (d-a);
	c2 -= c;
	// we assume we have zero length for an rectangle
	if (c2.magnitude() < 1e-4)  // TODO : should this be a relative error? suppose we have a polygon of size 1 mm x 1 mm, then any polygon will be a rectangle
		m_type = T_Rectangle;
}


bool Polygon3D::isSimplePolygon() {
	std::vector<IBK::Line>	lines;
	for (unsigned int i=0, vertexCount = m_vertexes.size(); i<vertexCount; ++i) {
		lines.emplace_back(
					IBK::Line(
					IBK::point2D<double>(
								  m_vertexes[i].m_x,
								  m_vertexes[i].m_y),
					IBK::point2D<double>(
								  m_vertexes[(i+1) % vertexCount].m_x,
								  m_vertexes[(i+1) % vertexCount].m_y))
				);
	}
	if (lines.size()<4)
		return true;
	for (unsigned int i=0; i<lines.size();++i) {
		for (unsigned int j=0; j<lines.size()-2; ++j) {
			unsigned int k1 = (i+1)%lines.size();
			unsigned int k2 = (i-1);
			if(i==0)
				k2 = lines.size()-1;
			if(i==j || k1 == j || k2 == j )
				continue;
			//int k = (i+j+2)%lines.size();
			IBK::point2D<double> p;
			if (lines[i].intersects(lines[j], p))
				return false;
		}
	}

	return true;
}


void Polygon3D::eleminateColinearPts() {
	IBKMK::eleminateColinearPoints(m_vertexes);
}


void Polygon3D::updateLocalCoordinateSystem() {
	m_normal = IBKMK::Vector3D(0,0,0);
	if (m_vertexes.size() < 3)
		return;

	// We define our normal via the winding order of the polygon.
	// Since our polygon may be concave (i.e. have dents), we cannot
	// just pick any point and compute the normal via the adjacent edge vectors.
	// Instead, we first calculate the normal vector based on the first two edges.
	// Then, we loop around the entire polygon, compute the normal vectors at
	// each vertex and compare it with the first. If pointing in the same direction,
	// we count up, otherwise down. The direction with the most normal vectors wins
	// and will become our polygon's normal vector.

	// calculate normal with first 3 points
	m_localX = m_vertexes[1] - m_vertexes[0];
	IBKMK::Vector3D y = m_vertexes.back() - m_vertexes[0];
	IBKMK::Vector3D n;
	m_localX.crossProduct(y, n);
	if (n.magnitude() < 1e-9)
		return; // invalid vertex input
	n.normalize();

	int sameDirectionCount = 0;

	// now process all other points and generate their normal vectors as well
	for (unsigned int i=1; i<m_vertexes.size(); ++i) {
		IBKMK::Vector3D vx = m_vertexes[(i+1) % m_vertexes.size()] - m_vertexes[i];
		IBKMK::Vector3D vy = m_vertexes[i-1] - m_vertexes[i];
		IBKMK::Vector3D vn;
		vx.crossProduct(vy, vn);
		if (vn.magnitude() < 1e-9)
			return; // invalid vertex input
		vn.normalize();
		// adding reference normal to current vertexes normal and checking magnitude works
		if ((vn + n).magnitude() > 1) // can be 0 or 2, so comparing against 1 is good even for rounding errors
			++sameDirectionCount;
		else
			--sameDirectionCount;
	}

	if (sameDirectionCount < 0) {
		// invert our normal vector
		n *= -1;
	}

	// save-guard against degenerate polygons (i.e. all points close to each other or whatever error may cause
	// the normal vector to have near zero magnitude... this may happen for extremely small polygons, when
	// the x and y vector lengths are less than 1 mm in length).
	m_normal = n;
	// now compute local Y axis
	n.crossProduct(m_localX, m_localY);
	// normalize localX and localY
	m_localX.normalize();
	m_localY.normalize();
}


void Polygon3D::setVertexes(const std::vector<IBKMK::Vector3D> & vertexes) {
	m_vertexes = vertexes;
	checkPolygon(); // if we have a triangle/rectangle, this is detected here
}

} // namespace VICUS

