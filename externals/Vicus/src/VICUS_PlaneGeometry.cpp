#include "VICUS_PlaneGeometry.h"

#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>

#include <NANDRAD_Utilities.h>

#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <QPolygonF>
#include <QVector2D>

#include <tinyxml.h>

namespace VICUS {

/* Solves equation system with Cramer's rule:
	 a x + c y = e
	 b x + d y = f
*/
static bool solve(double a, double b, double c,  double d,  double e,  double f, double & x, double & y) {
	double det = a*d - b*c;
	if (det == 0.)
		return false;

	x = (e*d - c*f)/det;
	y = (a*f - e*b)/det;
	return true;
}


/* Computes the coordinates x, y of a point 'p' in a plane spanned by vectors a and b from a point 'offset', where rhs = p-offset.
	The computed plane coordinates are stored in variables x and y (the factors for vectors a and b, respectively).
	If no solution could be found (only possible if a and b are collinear or one of the vectors has length 0?),
	the function returns false.

	Note: when the point p is not in the plane, this function will still get a valid result.
*/
static bool planeCoordinates(const IBKMK::Vector3D & offset, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b,
							 const IBKMK::Vector3D & v, double & x, double & y)
{
	// We have 3 equations, but only two unknowns - so we have 3 different options to compute them.
	// Some of them may fail, so we try them all.

	const IBKMK::Vector3D & rhs = v-offset;
	// rows 1 and 2
	bool success = solve(a.m_x, a.m_y, b.m_x, b.m_y, rhs.m_x, rhs.m_y, x, y);
	if (!success)
		// rows 1 and 3
		success = solve(a.m_x, a.m_z, b.m_x, b.m_z, rhs.m_x, rhs.m_z, x, y);
	if (!success)
		// rows 2 and 3
		success = solve(a.m_y, a.m_z, b.m_y, b.m_z, rhs.m_y, rhs.m_z, x, y);
	if (!success)
		return false;

	// check that the point was indeed in the plane
	IBKMK::Vector3D v2 = offset + x*a + y*b;
	v2 -= v;
	if (v2.magnitude() > 1e-4)
		return false;
	return true;
}


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
	unsigned int polySize = poly.size();
	for (size_t i=0; i<polySize; ++i) {
		t *= crossProdTest(point, poly.value(i), poly.value((i+1)%polySize));//  m_polyline[(i+1)%m_polyline.size()]);
		if(t==0)
			break;
	}

	return  t;
}


// *** PlaneGeometry ***



PlaneGeometry::PlaneGeometry(PlaneGeometry::type_t t,
							 const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) :
	m_type(t),
	m_vertexes({a,b,c})
{
	if (m_type == T_Rectangle) {
		// third vertex is actually point d of the rectangle
		m_vertexes.push_back(m_vertexes.back());
		// c = a + (b-a) + (d-a) = b + (d - a)
		m_vertexes[2] = m_vertexes[1] + (m_vertexes[3]-m_vertexes[0]);
	}
	computeGeometry();
}


void PlaneGeometry::readXML(const TiXmlElement * element) {
	readXMLPrivate(element);
	computeGeometry();
}


TiXmlElement * PlaneGeometry::writeXML(TiXmlElement * parent) const {
	if (*this != PlaneGeometry())
		return writeXMLPrivate(parent);
	else
		return nullptr;
}


void PlaneGeometry::addVertex(const QPointF & v) {
	m_polygon.append(v);
	m_type = T_Polygon; // assume the worst
	// compute 3D coordinates
	computeGeometry(); // if we have a triangle/rectangle, this is detected here
}


void PlaneGeometry::addVertex(const IBKMK::Vector3D & v) {
	m_vertexes.push_back(v);
	m_type = T_Polygon; // assume the worst
	computeGeometry(); // if we have a triangle/rectangle, this is detected here
}


void PlaneGeometry::removeVertex(unsigned int idx) {
	Q_ASSERT(idx < m_vertexes.size());
	m_vertexes.erase(m_vertexes.begin()+idx);
	m_type = T_Polygon; // assume the worst
	computeGeometry(); // if we have a triangle/rectangle, this is detected here
}


void PlaneGeometry::computeGeometry() {
	m_triangles.clear();
	// try to simplify polygon to internal rectangle/parallelogram definition
	// this may change m_type to Rectangle or Triangle and subsequently speed up operations
	simplify();
	eleminateColinearPts();
	updateNormal();
	if (!isValid())
		return;
	updatePolygon();
	triangulate();
}


void PlaneGeometry::simplify() {
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
	if (c2.magnitude() < 1e-4) {
		m_type = T_Rectangle;
	}
}


void PlaneGeometry::updatePolygon() {
	FUNCID(PlaneGeometry::updatePolygon);

	//first clear the old polyline
	m_polygon.clear();

	/// \todo Dirk: Add check that m_vertexes[1] != m_vertexes[0]

	// x-axis vector in plane
	m_localX = (m_vertexes[1] - m_vertexes[0]);
	// y-axis vector in plane
	m_normal.crossProduct(m_localX, m_localY);

	// first point is v0 = origin
	m_polygon.append(QPointF(0,0));
	// second point is v1 at (1,0), since v1-v0 is the vX vector
	m_polygon.append(QPointF(1,0));

	// now process all other points
	for (unsigned int i=2; i<m_vertexes.size(); ++i) {
		const IBKMK::Vector3D & v = m_vertexes[i];
		double x,y;
		/// \todo improve this - by simply calling planeCoordinates we
		///       redo the same stuff several times for the same plane.
		///       We should use a function that passes vX, vY, offset and then
		///       a vector with v,x,y to process.
		if (planeCoordinates(m_vertexes[0], m_localX, m_localY, v, x, y)) {
			m_polygon.append(QPointF(x,y));
		}
		else {
			// Throw an exception if point is outside plane
			throw IBK::Exception("Point is outside plane.", FUNC_ID);
		}
	}
}

QPolygonF PlaneGeometry::eleminateColinearPts(bool overrideMemberVar){

	const double eps = 1e-4;
	unsigned int polySize = m_polygon.size();
	QPolygonF newPoly;
	for(unsigned int idx=0; idx<polySize; ++idx){
		unsigned int idx0 = idx-1;
		if(idx==0)
			idx0 = polySize-1;

		QVector2D a(m_polygon[idx0]-m_polygon[idx]);
		QVector2D b(m_polygon[(idx+1)%polySize]-m_polygon[idx]);
		double cosAngle = QVector2D::dotProduct(a.normalized() , b.normalized());

		// points are not colinear add point to new polygon
		if(cosAngle > -1+eps && cosAngle < 1-eps){
			newPoly << m_polygon.value(idx);
		}
	}
	if(overrideMemberVar)
		m_polygon = newPoly;
	return  newPoly;
}


void PlaneGeometry::triangulate() {
	FUNCID(PlaneGeometry::triangulate);
	Q_ASSERT(m_vertexes.size() >= 3);
	Q_ASSERT(m_polygon.size() == m_vertexes.size());

	bool isDrawMode = true;

	const double eps = 1e-4;
	m_triangles.clear();
	switch (m_type) {

		case T_Triangle :
			m_triangles.push_back( triangle_t(0, 1, 2) );
			break;

		case T_Rectangle :
			// TODO : there might be a faster way for rectangles, but for now
			//        we use the same triangulation algorithm as for polygons
		case T_Polygon : {
			//here the index is stored which is already taken into account
			std::set<unsigned int> usedIdx;
			std::vector<std::vector<unsigned int>>	trisIndices;


			// create a polygon without colinear points
			QPolygonF polygon = m_polygon;

			if(false){
				polygon.clear();
				polygon << QPointF(0,0);
				polygon << QPointF(1,0);
				polygon << QPointF(1,1);
				polygon << QPointF(0,1);
				polygon << QPointF(0,2);
				polygon << QPointF(-1,2);
				polygon << QPointF(-1,1);
				polygon << QPointF(-1,0);
//				polygon << QPointF(-2,0);
//				polygon << QPointF(-2,-1);
//				polygon << QPointF(-1,-1);
//				polygon << QPointF(0,-1);
				m_polygon = polygon;
			}

			polygon = eleminateColinearPts(true);
			//m_polygon = polygon;

			//if only 3 points --> it is a triangle
			//no calculation needed
			/// TODO Dirk umarbeiten da es polygone geben könnte die am ende nur noch 3 punkte haben
			/// aber nicht die ersten drei
			if(m_vertexes.size() == 3){
				m_triangles.push_back( triangle_t(0, 1, 2) );
				return;
			}

			///TODO Dirk endlosschleife abfangen wenn kein erfolg war

			std::set<unsigned int>			oldTriIdx;			//index in the right order for the triangle from the previours step

			unsigned int idx = 0;
			for(;;){

				if(idx >= polygon.size())
					idx = 0;

				QPolygonF triangle;
				//check if there are enough points left
				if(usedIdx.size()>=polygon.size()-2)
					break;

				std::set<unsigned int>			triIdx;			//index that used for triangle as set for quick access
				std::vector<unsigned int>		triIdxVec;		//index in the right order for the triangle

				//build a triangle with unused points
				unsigned int idx2 = idx;
				for(;;++idx2){
					if(idx2 >= polygon.size())
						idx2 = 0;

					//check if index is not already used
					if(usedIdx.find(idx2) == usedIdx.end()){
						triangle <<	polygon.value(idx2);
						triIdx.insert(idx2);
						triIdxVec.push_back(idx2);
					}

					//need three for fruther calculation
					if(triangle.size() != 3)
						continue;
					//now we have a three points

					QVector2D a(polygon[triIdxVec[1]]-polygon[triIdxVec[0]]);
					QVector2D b(polygon[triIdxVec[2]]-polygon[triIdxVec[0]]);
					double cosAngle = QVector2D::dotProduct(a.normalized() , b.normalized());
					//double angleDeg = std::acos(cosAngle)*180/PI;

					//check if last point cotains triangle (only in DrawMode)
					//					if(isDrawMode &&
					//							triIdx.find(m_polygon.size()-1) != triIdx.end() &&
					//							triIdx.find(0) != triIdx.end() &&
					//							false){
					//						//triangle is not the same as last one
					//						//check for a other triangle
					//						if(triIdx != oldTriIdx){
					//							oldTriIdx = triIdx;
					//							triIdx.clear();
					//							triangle.clear();
					//							triIdxVec.clear();
					//							idx2=0;
					//							continue;
					//						}
					//						else {
					//							//oldTriIdx = triIdx;
					//						}

					//					}


					//					if(triIdxVec.size()!=3)
					//						continue;


					//check if points are colinear
					if(cosAngle < -1+eps || cosAngle > 1-eps){

						idx2 -= 2;
						idx = triIdxVec[1];

						if(oldTriIdx== triIdx){
							//found again some triangle --> error
							//IBK::Exception(IBK::FormatString("Polygon is not valid."), FUNC_ID);
							///TODO Andreas falls er fertig is Exception werfen oder Weiterbehandlung
							return;
						}
						oldTriIdx = triIdx;


						//erase first triangle point and check for another one
						if(false){
							triIdx.erase(triIdxVec[0]);
							triangle.takeAt(0);
							triIdxVec.erase(triIdxVec.begin());
						}
						else {
							triIdx.clear();
							triangle.clear();
							triIdxVec.clear();
						}
					}
					else {
						//	found a triangle
						break;
					}
				}

				if(triIdx.size() == 3){
					if(oldTriIdx== triIdx){
						//found again some triangle --> error
						//IBK::Exception(IBK::FormatString("Polygon is not valid."), FUNC_ID);
						///TODO Andreas falls er fertig is Exception werfen oder Weiterbehandlung
						return;
					}
					oldTriIdx = triIdx;
				}

				// fast validity check: invalid triangles whose hypothenusis is completely outside polygon
				// construct a random testpoint at hypothenusis
				QPointF testPoint(triangle.value(0) + (triangle.value(1)-triangle.value(0) + triangle.value(2)-triangle.value(1)) *0.5);

				//check if the test point is outside the plane polygon
				if(pointInPolygon(testPoint, polygon) == -1){
					idx = triIdxVec[1];
					continue;
				}

				// fast check:
				//check if no other point is situated inside the triangle
				unsigned int i=0;
				for (;i<polygon.size(); ++i) {
					//skip points that are in the triangle
					if(triIdx.find(i) != triIdx.end())
						continue;

					//check if point of the polygon is inside the triangle
					if(pointInPolygon(polygon.value(i), triangle) != -1){
						//if(triangle.containsPoint(polygon.value(i), Qt::FillRule::OddEvenFill)){
						//triangle invalid
						break;
					}
				}
				// minimum one point of polygon is inside triangle, loop was broken before end
				if(i < polygon.size()){
					idx = triIdxVec[1];
					continue;
				}

				// advanced check
				// intersect polygon and triangle in order to find invalid overlaping regions
				QPolygonF intersectionPolyAndTriangle = polygon.intersected(triangle);

				// triangle is outside polygon
				if(intersectionPolyAndTriangle.empty()){
					idx = triIdxVec[1];
					continue;
				}

				// QPolygonF is a closed polyline so skip last point
				//lastpoint = firstpoint
				intersectionPolyAndTriangle.erase(intersectionPolyAndTriangle.end()-1);

				//intersection between polygon and triangle must be the triangle
				if(intersectionPolyAndTriangle.size() != 3){
					idx = triIdxVec[1];
					continue;
				}

				// count common points between intersection and original triangle
				unsigned int counter=0;
				for (auto pTri1 : triangle) {
					for (auto pTri2 : intersectionPolyAndTriangle) {
						if(pTri1 == pTri2)
							++counter;
					}
				}

				//all points of intersected triangle must match the original triangle
				if(counter != 3){
					idx = triIdxVec[1];
					continue;
				}

				//found valid triangle
				m_triangles.push_back(triangle_t(triIdxVec[0],triIdxVec[1],triIdxVec[2]));
				idx = triIdxVec[2];
				usedIdx.insert(triIdxVec[1]);
			}
		}
			break;	//case

		case NUM_T : ; // shouldn't happen
	}

}


void PlaneGeometry::updateNormal() {
	m_normal = IBKMK::Vector3D(0,0,0);
	if (m_vertexes.size() < 3)
		return;

	// loop around vertexes and try to find at least three vertexes with existing cross-product
	for (unsigned int i=0; i<m_vertexes.size()-2; ++i) {
		IBKMK::Vector3D ba = m_vertexes[2+i] - m_vertexes[1+i];
		IBKMK::Vector3D ca = m_vertexes[0+i] - m_vertexes[1+i];
		IBKMK::Vector3D n;
		ba.crossProduct(ca, n);
		if (n.magnitude() > 1e-4) {
			m_normal = n;
			m_normal.normalize();
			return; // found a normal vector
		}
	}

}

void PlaneGeometry::setVertexes(const std::vector<IBKMK::Vector3D> & vertexes) {
	m_vertexes = vertexes;
	computeGeometry();
}


bool PlaneGeometry::intersectsLine(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & d, IBKMK::Vector3D & intersectionPoint,
								   double & dist, bool hitBackfacingPlanes, bool endlessPlane) const
{
	IBK_ASSERT(m_vertexes.size() >= 3);
	// first the normal test

	double d_dot_normal = d.scalarProduct(m_normal);
	double angle = d_dot_normal/d.magnitude();
	// line parallel to plane? no intersection
	if (angle < 1e-8 && angle > -1e-8)
		return false;

	// Condition 1: same direction of normal vectors?
	if (!hitBackfacingPlanes && angle >= 0)
		return false; // no intersection possible

	const IBKMK::Vector3D & offset = m_vertexes[0];

	double t = (offset - p1).scalarProduct(m_normal) / d_dot_normal;

	// Condition 2: outside viewing range?
	if (t < 0 || t > 1)
		return false;

	// now determine location on plane
	IBKMK::Vector3D x0 = p1 + t*d;

	// plane is endless - return intersection point and normalized distance t
	if (endlessPlane) {
		intersectionPoint = x0;
		dist = t;
		return true;
	}

	// test if intersection point is inside our plane
	// we have a specialized variant for triangles and rectangles

	switch (m_type) {
		case T_Triangle :
		case T_Rectangle : {

			// we have three possible ways to get the intersection point, try them all until we succeed
			const IBKMK::Vector3D & a = m_vertexes[1] - m_vertexes[0];
			const IBKMK::Vector3D & b = m_vertexes.back() - m_vertexes[0];
			double x,y;
			if (!planeCoordinates(offset, a, b, x0, x, y))
				return false;

			if (m_type == T_Triangle && x >= 0 && x+y <= 1 && y >= 0) {
				intersectionPoint = x0;
				dist = t;
				return true;
			}
			else if (m_type == T_Rectangle && x >= 0 && x <= 1 && y >= 0 && y <= 1) {
				intersectionPoint = x0;
				dist = t;
				return true;
			}

		} break;

		case T_Polygon : {
			double x,y;
			if (!planeCoordinates(offset, m_localX, m_localY, x0, x, y))
				return false;
			// process all triangles and perform the test for each triangle
			return (pointInPolygon(QPointF(x,y), m_polygon) != -1);
			//			for (const triangle_t & tr : m_triangles) {
			//				double x,y;
			//				const IBKMK::Vector3D & a = m_vertexes[tr.c] - m_vertexes[tr.b];
			//				const IBKMK::Vector3D & b = m_vertexes[tr.a] - m_vertexes[tr.b];
			//				if (planeCoordinates(x0,a,b, m_vertexes[tr.b],x,y)) {
			//					if (x >= 0 && x+y <= 1 && y >= 0) {
			//						intersectionPoint = x0;
			//						dist = t;
			//						return true;
			//					}
			//				}
			//			}
		} break;
	}

	return false;

}


bool PlaneGeometry::operator!=(const PlaneGeometry & other) const {
	if (m_type != other.m_type) return true;
	if (m_vertexes != other.m_vertexes) return true;
	return false;
}


void PlaneGeometry::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(PlaneGeometry::readXMLPrivate);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "type")
				try {
				m_type = (type_t)KeywordList::Enumeration("PlaneGeometry::type_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
										  IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			attrib = attrib->Next();
		}
		// read data
		NANDRAD::readVector3D(element, "PlaneGeometry", m_vertexes);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'PlaneGeometry' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'PlaneGeometry' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement * PlaneGeometry::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("PlaneGeometry");
	parent->LinkEndChild(e);

	if (m_type != NUM_T)
		e->SetAttribute("type", KeywordList::Keyword("PlaneGeometry::type_t",  m_type));
	std::stringstream vals;
	for (unsigned int i=0; i<m_vertexes.size(); ++i) {
		vals << m_vertexes[i].m_x << " " << m_vertexes[i].m_y << " " << m_vertexes[i].m_z;
		if (i<m_vertexes.size()-1)  vals << ", ";
	}
	TiXmlText * text = new TiXmlText( vals.str() );
	e->LinkEndChild( text );
	return e;
}

} // namespace VICUS
