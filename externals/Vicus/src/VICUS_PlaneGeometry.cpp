#include "VICUS_PlaneGeometry.h"

#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>

#include <NANDRAD_Utilities.h>

#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

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
static bool planeCoordinates(const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & rhs, double & x, double & y) {
	// We have 3 equations, but only two unknowns - so we have 3 different options to compute them.
	// Some of them may fail, so we try them all.

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
	return true;
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


PlaneGeometry::PlaneGeometry() {
}


PlaneGeometry::PlaneGeometry(PlaneGeometry::type_t t,
							 const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) :
	m_type(t),
	m_vertexes({a,b,c})
{
	computeGeometry();
}


void PlaneGeometry::addVertex(const IBKMK::Vector3D & v) {
	m_vertexes.push_back(v);
	computeGeometry();
}


void PlaneGeometry::computeGeometry() {
	m_triangles.clear();
	// try to simplify polygon to internal rectangle/parallelogram definition
	// this may change m_type to Rectangle or Triangle and subsequently speed up operations
	simplify();
	updateNormal();
	if (!isValid())
		return;
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
	if (c2.magnitudeSquared() < 1e-4) {
		m_type = T_Rectangle;
	}
}


void PlaneGeometry::triangulate() {
	m_triangles.clear();
	switch (m_type) {

		case T_Triangle :
			m_triangles.push_back( TriangleVertexNumbers(0, 1, 2) );
		break;

		case T_Rectangle : {
			// We might have a concave rectangle, i.e. where one diagonal is outside
			// the rectangle (or aligned with one of the edges).
			// Easiest way to find this is to check if any of the angles between two adjacent
			// edges is > 180°. However, we cannot do this easily, so we take the Anne's idea.
			// We process two neighboring vertices.
			// For each vertex, we span a plane (vectors to left and right neighboring vertexes).
			// Then, we compute the vector factors to the 4th vertex - if either of them is
			// negative, the point lies outside the rectangle spanned by the vertexes and hence, we
			// need to use the other diagonal to split the triangle.

			// Try vertex 0 first
			IBKMK::Vector3D a;


			// none found, just split up at vertex 0..2
			m_triangles.push_back( TriangleVertexNumbers(0, 1, 2) );
			m_triangles.push_back( TriangleVertexNumbers(0, 2, 3) );
		} break;

		case T_Polygon : {
			//
		}

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
		if (n.magnitudeSquared() > 1e-4) {
			m_normal = n;
			return; // found a normal vector
		}
	}

}


bool PlaneGeometry::intersectsLine(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & d, IBKMK::Vector3D & intersectionPoint,
								   double & dist, bool hitBackfacingPlanes, bool endlessPlane) const
{
	IBK_ASSERT(m_vertexes.size() >= 3);
	// first the normal test

	double d_cross_normal = d.scalarProduct(m_normal);
	double angle = d_cross_normal/d.magnitudeSquared();
	// line parallel to plane? no intersection
	if (angle < 1e-8 && angle > -1e-8)
		return false;

	// Condition 1: same direction of normal vectors?
	if (!hitBackfacingPlanes && angle >= 0)
		return false; // no intersection possible

	const IBKMK::Vector3D & offset = m_vertexes[0];

	double t = (offset - p1).scalarProduct(m_normal) / d_cross_normal;

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
			IBKMK::Vector3D rhs = x0 - offset; // right hand side of equation system:  a * x  +  b * y = (x - offset)
			double x,y;
			if (!planeCoordinates(a, b, rhs, x, y))
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
			// process all triangles and perform the test for each triangle
			for (const TriangleVertexNumbers & tr : m_triangles) {
				double x,y;
				const IBKMK::Vector3D & a = m_vertexes[tr.c] - m_vertexes[tr.b];
				const IBKMK::Vector3D & b = m_vertexes[tr.a] - m_vertexes[tr.b];
				IBKMK::Vector3D rhs = x0 - m_vertexes[tr.b]; // right hand side of equation system:  a * x  +  b * y = (x - offset)
				if (planeCoordinates(a,b,rhs,x,y)) {
					if (x >= 0 && x+y <= 1 && y >= 0) {
						intersectionPoint = x0;
						dist = t;
						return true;
					}
				}
			}
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
