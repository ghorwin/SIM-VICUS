/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#include "IBKMK_Polygon3D.h"

#include <IBK_Line.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include "IBKMK_3DCalculations.h"

namespace IBKMK {

Polygon3D::Polygon3D(Polygon2D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) {
	std::vector<IBKMK::Vector3D> verts;
	verts.push_back(a);
	verts.push_back(b);
	verts.push_back(c);
	if (t == Polygon2D::T_Rectangle) {
		// third vertex is actually point d of the rectangle, so we first set vertex[3] = vertex[2],
		// then compute vertex [c] again
		verts.push_back(verts.back());
		// c = a + (b-a) + (d-a) = b + (d - a)
		verts[2] = verts[1] + (verts[3]-verts[0]);
	}
	// now generate a Polygon3D
	(*this) = Polygon3D(verts);
}


Polygon3D::Polygon3D(const Polygon2D & polyline, const IBKMK::Vector3D & offset,
					 const IBKMK::Vector3D & normal, const IBKMK::Vector3D & localX) :
	m_offset(offset), m_polyline(polyline)
{
	FUNCID(Polygon3D::Polygon3D);
	// first point must be 0,0
	if (!polyline.isValid()) {
		m_valid = false;
		return;
	}
	if (polyline.vertexes()[0] != IBKMK::Vector2D(0,0)) {
		IBK_Message("First point of polyline must be 0,0!", IBK::MSG_ERROR, FUNC_ID);
		m_valid = false;
		return;
	}
	try {
		m_valid = true; // assume polygon is valid
		setRotation(normal, localX); // also sets the m_dirty flag
	} catch (...) {
		m_valid = false;
	}
}


Polygon3D::Polygon3D(const std::vector<IBKMK::Vector3D> & vertexes) {
	setVertexes(vertexes, false); // no automatic healing here! We want to know if vertexes are bad.
	// mark 3D vertexes as dirty
	m_dirty = true;
}


bool Polygon3D::setVertexes(const std::vector<Vector3D> & vertexes, bool heal) {
	FUNCID(Polygon3D::setVertexes);

	// we construct a polygon from points by first eliminating collinear points, then checking

	m_valid = false;
	m_polyline.clear();
	if (vertexes.size() < 3)
		return false;

	// eliminate colliniear points in temporary vector
	std::vector<IBKMK::Vector3D> verts(vertexes);
	IBKMK::eliminateCollinearPoints(verts);

	updateLocalCoordinateSystem(verts);
	// we need 3 vertexes (not collinear) to continue and a valid normal vector!
	if (verts.size() < 3 || m_normal == IBKMK::Vector3D(0,0,0))
		return false;

	// we now have a valid local coordinate sytstem, set our original to the first vertex of the remaining
	// vertexes, so that the polygon has always 0,0 as first point
	update2DPolyline(verts);

	// polygon must not be winding into itself, otherwise triangulation would not be meaningful
	m_valid = m_polyline.isValid() && m_polyline.isSimplePolygon();

	// if our polyline is not valid and we are requested to attempt healing, do this now
	if (heal && !m_polyline.isValid()) {
		IBK::IBK_Message("Attempting healing of polygon 3D.", IBK::MSG_DEBUG, FUNC_ID, IBK::VL_INFO);

		// we take a vector to hold our deviations, i.e. the sum of the vertical deviations from the plane.
		std::vector<double> deviations (verts.size(), 0);
		// create a vector to hold the projected points for each of the plane variants
		std::vector<std::vector<IBKMK::Vector3D> > projectedPoints ( verts.size(), std::vector<IBKMK::Vector3D> ( verts.size(), IBKMK::Vector3D (0,0,0) ) );

		// we iterate through all points and construct planes
		double smallestDeviation = std::numeric_limits<double>::max();
		unsigned int bestPlaneIndex = (unsigned int)-1;
		for (unsigned int i = 0, count = verts.size(); i<count; ++i ) {

			// select point i  as offset
			const IBKMK::Vector3D & offset = verts[i];

			// define plane through vectors a and b
			const IBKMK::Vector3D & a = verts[(i + 1)         % count] - offset;
			const IBKMK::Vector3D & b = verts[(i - 1 + count) % count] - offset;

			// we now iterate again through all points of the polygon and
			// sum up the distances of all points to their projection point
			for (unsigned int j = 0; j<count; ++j ) {

				// offset point?
				if ( i == j ) {
					projectedPoints[i][j] = offset;
					continue;
				}

				// we take the current point
				const IBKMK::Vector3D & vertex = verts[j];

				// we find our projected points onto the plane
				double x, y;
				// allow a fairly large tolerance here
				if (IBKMK::planeCoordinates(offset, a, b, vertex, x, y, 1e-2)) {

					// now we construct our projected points and find the deviation between the original points
					// and their projection
					projectedPoints[i][j] = offset + a*x + b*y;

					// add up the distance between original vertex and projected point
					// Note: if we add the square of the distances, we still get the maximum deviation, but avoid
					//       the expensive square-root calculation
					deviations[i] += (projectedPoints[i][j] - vertex).magnitudeSquared();
				}
				else {
					// if we cannot find a valid project point for any reason, store the original point and set
					// a very large distance
					projectedPoints[i][j] = vertex;
					deviations[i] += 100; // this will effectively eliminate this plane as option
				}
			}

			// remember plane index if new deviation is smallest
			if (deviations[i] < smallestDeviation) {
				bestPlaneIndex = i;
				smallestDeviation = deviations[i];
			}
		}

		// if one of the points is soo far out of the polygon, that fixing is not even possible, we note that as error
		if (smallestDeviation > 100) {
			std::stringstream strm;
			for (unsigned int i = 0, count = verts.size(); i<count; ++i )
				strm << verts[i].toString() << (i+1<count ? ", " : "");
			IBK::IBK_Message("Cannot fix polygon: " + strm.str(), IBK::MSG_DEBUG, FUNC_ID, IBK::VL_INFO);
			return false; // polygon remains invalid
		}

		// take the best vertex set and use it for the polygon
		setVertexes(projectedPoints[bestPlaneIndex], false); // Mind: we call our function again, but this time without healing to avoid looping
	}
	m_dirty = true;
	return m_valid;
}


// Comparison operator !=
bool Polygon3D::operator!=(const Polygon3D &other) const {

	if (m_valid != other.m_valid ||
		m_normal != other.m_normal ||
		m_offset != other.m_offset ||
		m_localX != other.m_localX ||
		m_polyline != other.m_polyline)
	{
		return true;
	}

	return false;
}


const std::vector<Vector3D> & Polygon3D::vertexes() const {
	FUNCID(Polygon3D::vertexes);
	if(m_polyline.vertexes().empty())
		throw IBK::Exception("Polyline does not contain any vertexes!", FUNC_ID);

	if (m_dirty) {
		// recompute 3D vertex cache

		const std::vector<IBKMK::Vector2D> &polylineVertexes = m_polyline.vertexes();
		m_vertexes.resize(polylineVertexes.size());
		m_vertexes[0] = m_offset;
		for (unsigned int i=1; i<m_vertexes.size(); ++i)
			m_vertexes[i] = m_offset + m_localX * polylineVertexes[i].m_x + m_localY * polylineVertexes[i].m_y;

		m_dirty = false;
	}
	return m_vertexes;
}

const std::vector<Vector3D> &Polygon3D::rawVertexes() const {
	return m_vertexes;
}


// *** Transformation Functions ***

void Polygon3D::setRotation(const IBKMK::Vector3D & normal, const IBKMK::Vector3D & xAxis) {
	FUNCID(Polygon3D::setRotation);

	if (!IBK::nearly_equal<4>(normal.magnitudeSquared(), 1.0))
		throw IBK::Exception("Normal vector does not have unit length!", FUNC_ID);
	if (!IBK::nearly_equal<4>(xAxis.magnitudeSquared(), 1.0))
		throw IBK::Exception("xAxis vector does not have unit length!", FUNC_ID);
	// check that the vectors are (nearly) orthogonal
	double sp = normal.scalarProduct(xAxis);
	if (!IBK::nearly_equal<4>(sp, 0.0))
		throw IBK::Exception("Normal and xAxis vectors must be orthogonal!", FUNC_ID);

	// we only modify our vectors if all input data is correct - hence we ensure validity of the polygon
	m_normal = normal;
	m_localX = xAxis;
	normal.crossProduct(xAxis, m_localY); // Y = N x X - right-handed coordinate system
	m_localY.normalize();
	m_dirty = true; // mark 3D vertexes as dirty
}


void Polygon3D::flip() {
	IBK_ASSERT(isValid());
	m_normal = -1.0*m_normal;
	// we need to swap x and y axes to keep right-handed coordinate system
	std::swap(m_localX, m_localY);

	// we also need to swap x and y coordinates of all polygon2D points
	std::vector<IBKMK::Vector2D>		vertexes2D = m_polyline.vertexes();
	std::vector<IBKMK::Vector2D>		vertexes2DNew;
	for (IBKMK::Vector2D & v : vertexes2D)
		std::swap(v.m_x, v.m_y);

	// attention update offset
	m_offset = m_offset + m_localX * vertexes2D.back().m_x + m_localY * vertexes2D.back().m_y;

	// so now we have to update all points in 2D to new offset point
	IBKMK::Vector2D offset2D = vertexes2D.back() - vertexes2D.front();

	// to comply with the flipped surface normal we also have to reverse the order 2D points
	for (unsigned int i=vertexes2D.size(); i>0; --i)
		vertexes2DNew.push_back(vertexes2D[i-1]-offset2D);

	m_polyline.setVertexes(vertexes2DNew);
	m_dirty = true;
	vertexes();
}


// *** Calculation Functions ***


IBKMK::Vector3D Polygon3D::centerPoint() const {
	FUNCID(Polygon3D::centerPoint);
	if (!isValid())
		throw IBK::Exception("Invalid polygon.", FUNC_ID);

	size_t counter=0;
	IBKMK::Vector3D vCenter;

	for (const IBKMK::Vector3D & v : vertexes()) {
		vCenter += v;
		++counter;
	}
	vCenter/=static_cast<double>(counter);

	return vCenter;
}


void Polygon3D::boundingBox(Vector3D & lowerValues, Vector3D & upperValues) const {
	FUNCID(Polygon3D::boundingBox);
	if (!isValid())
		throw IBK::Exception("Invalid polygon.", FUNC_ID);
	// Note: do not access m_vertexes directly, as this array may be dirty
	const std::vector<Vector3D> & points = vertexes();
	// initialize bounding box with first point
	lowerValues = points[0];
	upperValues = points[0];
	for (unsigned int i=1; i<points.size(); ++i)
		IBKMK::enlargeBoundingBox(points[i], lowerValues, upperValues);
}


void Polygon3D::enlargeBoundingBox(Vector3D & lowerValues, Vector3D & upperValues) const {
	FUNCID(Polygon3D::enlargeBoundingBox);
	if (!isValid())
		throw IBK::Exception("Invalid polygon.", FUNC_ID);
	// Note: do not access m_vertexes directly, as this array may be dirty
	const std::vector<Vector3D> & points = vertexes();
	for (const IBKMK::Vector3D & v: points)
		IBKMK::enlargeBoundingBox(v, lowerValues, upperValues);
}



// *** PRIVATE MEMBER FUNCTIONS ***

bool Polygon3D::smallerVectZero(const IBKMK::Vector3D& vect) {
	if (vect.m_x < 0)
		return true;
	if (vect.m_x > 0)
		return false;
	if (vect.m_y < 0)
		return true;
	if (vect.m_y > 0)
		return false;
	if (vect.m_z < 0)
		return true;
	if (vect.m_z > 0)
		return false;
	return false;
}

IBKMK::Vector3D Polygon3D::computeNormal(const std::vector<IBKMK::Vector3D>& polygon) {
	FUNCID(Polygon3D::computeNormal);

	if (polygon.size() < 3)
		return IBKMK::Vector3D(1,0,0);

	IBKMK::Vector3D n(0,0,0);
	IBKMK::Vector3D e(0,0,0);

	for(unsigned int i=0; i<polygon.size(); ++i)
		e += polygon[i];

	e /= polygon.size();


	for(unsigned int i=0; i<polygon.size(); ++i) {
		unsigned int s = polygon.size();
		unsigned int j = (i + s - 1)%s;

		IBKMK::Vector3D v1 = polygon[j] - e;
		IBKMK::Vector3D v2 = polygon[i] - e;

		n += v1.crossProduct(v2).normalized();
	}

	if (n.magnitudeSquared() < 0.01) {

		n = Vector3D(0,0,0);

		for(unsigned int i=0; i<polygon.size()-1; ++i) {
			unsigned int s = polygon.size();
			unsigned int j = (i + s - 1)%s;

			IBKMK::Vector3D v1 = polygon[j] - e;
			IBKMK::Vector3D v2 = polygon[i] - e;

			n += v1.crossProduct(v2).normalized();
		}

		if (n.magnitudeSquared() > 0.01)
			return n.normalized();

		IBK::IBK_Message(IBK::FormatString("Start point:\t%1\t%2\t%3")
						 .arg(e.m_x)
						 .arg(e.m_y)
						 .arg(e.m_z), IBK::MSG_ERROR);
		for (unsigned int i=0; i<polygon.size(); ++i) {

			unsigned int s = polygon.size();
			unsigned int j = (i + s -1)%s;

			IBKMK::Vector3D v1 = polygon[j] - e;
			IBKMK::Vector3D v2 = polygon[i] - e;

			Vector3D ntest = v1.crossProduct(v2).normalized();

			IBK::IBK_Message(IBK::FormatString("Poly point %4:\t%1\t%2\t%3\t\tNormal:\t%5\t%6\t%7")
							 .arg(polygon[i].m_x)
							 .arg(polygon[i].m_y)
							 .arg(polygon[i].m_z)
							 .arg(i)
							 .arg(ntest.m_x)
							 .arg(ntest.m_y)
							 .arg(ntest.m_z),IBK::MSG_ERROR);
		}

		throw IBK::Exception(IBK::FormatString("Could not determine normal of polygon 3D."), FUNC_ID);
	}

	return n.normalized();
}

void Polygon3D::updateLocalCoordinateSystem(const std::vector<IBKMK::Vector3D> & verts) {
	// NOTE: DO NOT ACCESS m_vertexes IN THIS FUNCTION!

	m_normal = IBKMK::Vector3D(0,0,0);
	if (verts.size() < 3)
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
	m_localX = verts[1] - verts[0];
//	IBKMK::Vector3D y = verts.back() - verts[0];
	IBKMK::Vector3D n1 = computeNormal(verts);
//	IBKMK::Vector3D n2;

//	m_localX.crossProduct(y, n2);


//	if(n1 != n2) {
//		IBK::IBK_Message(IBK::FormatString("Normal 1 - X: %1 Y: %2 Z: %3")
//						 .arg(n1.m_x).arg(n1.m_y).arg(n1.m_z), IBK::MSG_WARNING);

//		IBK::IBK_Message(IBK::FormatString("Normal 2 - X: %1 Y: %2 Z: %3")
//						 .arg(n2.m_x).arg(n2.m_y).arg(n2.m_z), IBK::MSG_WARNING);
//	}

	// if we interpret n as area between y and localX vectors, this should
	// be reasonably large (> 1 mm2). If we, however, have a very small magnitude
	// the vectors y and localX are (nearly) collinear, which should have been prevented by
	// eliminateColliniarPoints() before.
	if (n1.magnitude() < 1e-9)
		return; // invalid vertex input
	//n2.normalize();

	/* das ist alt und kann weg da die richtung der normalen nicht immer richtig ist.
	 * das wird an anderer stelle entschieden

	int sameDirectionCount = 0;
	// now process all other points and generate their normal vectors as well
	for (unsigned int i=1; i<verts.size(); ++i) {
		IBKMK::Vector3D vx = verts[(i+1) % verts.size()] - verts[i];
		IBKMK::Vector3D vy = verts[i-1] - verts[i];
		IBKMK::Vector3D vn;
		vx.crossProduct(vy, vn);
		// again, we check for not collinear points here (see explanation above)
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
	*/

	// save-guard against degenerate polygons (i.e. all points close to each other or whatever error may cause
	// the normal vector to have near zero magnitude... this may happen for extremely small polygons, when
	// the x and y vector lengths are less than 1 mm in length).
	m_normal = n1;
	// now compute local Y axis
	n1.crossProduct(m_localX, m_localY);
	// normalize localX and localY
	m_localX.normalize();
	m_localY.normalize();
	// store first point as offset
	m_offset = verts[0];
}



void Polygon3D::update2DPolyline(const std::vector<Vector3D> & verts) {
	// NOTE: DO NOT ACCESS m_vertexes IN THIS FUNCTION!

	m_polyline.clear();
	IBK_ASSERT(verts.size() >= 3);

	std::vector<IBKMK::Vector2D> poly;
	poly.reserve(verts.size());

	// first point is v0 = origin
	poly.push_back( IBKMK::Vector2D(0,0) );
	const IBKMK::Vector3D & offset = verts[0];

	// now process all other points
	for (unsigned int i=1; i<verts.size(); ++i) {
		const IBKMK::Vector3D & v = verts[i];
		double x,y;
		if (IBKMK::planeCoordinates(offset, m_localX, m_localY, v, x, y)) {
			poly.push_back( IBKMK::Vector2D(x,y) );
		}
		else {
			return;
		}
	}
	// set polygon in polyline
	// Mind: this may lead to removal of points if two are close together
	//       and thus also cause the polygon to be invalid
	m_polyline.setVertexes(poly);
	bool valid = m_polyline.isValid() && m_polyline.isSimplePolygon();

	// check if normal is right areaSigned >= 0 than ok
	// if not calculate a new one ... also calculate new localY
	if(valid){
		double areaS2 = m_polyline.areaSigned(2);

		if(areaS2 <0){
			m_normal *= -1;
			m_localX.crossProduct(m_normal, m_localY);
		}
	}

}


} // namespace IBKMK

