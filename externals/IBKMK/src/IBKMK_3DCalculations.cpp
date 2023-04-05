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

#include "IBKMK_3DCalculations.h"
#include "IBKMK_2DCalculations.h"

#include "IBKMK_Vector3D.h"
#include <IBK_messages.h>

namespace IBKMK {


/* Solves equation system with Cramer's rule:
	 a x + c y = e
	 b x + d y = f
*/
static bool solve(double a, double b, double c,  double d,  double e,  double f, double & x, double & y) {
	double det = a*d - b*c;
	// Prevent division by very small numbers
	if (std::fabs(det) < 1e-4)
		return false;

	x = (e*d - c*f)/det;
	y = (a*f - e*b)/det;
	return true;
}


/* Computes the coordinates x, y of a point 'v' in a plane spanned by vectors a and b from a point 'offset', where rhs = p-offset.
	The computed plane coordinates are stored in variables x and y (the factors for vectors a and b, respectively).
	If no solution could be found (only possible if a and b are collinear or one of the vectors has length 0?),
	the function returns false.

	Note: when the point p is not in the plane, this function will still get a valid result.
*/
bool planeCoordinates(const Vector3D & offset, const Vector3D & a, const Vector3D & b,
							 const Vector3D & v, double & x, double & y, double tolerance, bool showWarings)
{
	FUNCID(IBKMK::planeCoordinates);
	// compute projection of vector v onto plane

	// scalar project of vector v to offset and normal vector of plane
	IBKMK::Vector3D n = a.crossProduct(b);
	n.normalize();

	// dist = length component of normal vector in vector (offset-v)
	double dist = (offset - v).scalarProduct(n);
	// move point v along normal vector with distance dist
	IBKMK::Vector3D v2 = v - dist*n;
	IBKMK::Vector3D v2offset = v2 - v;
	if (showWarings && v2offset.magnitude() > tolerance) {
		IBK::IBK_Message(IBK::FormatString("Distance between point (%1) and projection point (%2) is too large (distance = %3)!")
						 .arg(v.toString()).arg(v2.toString()).arg(v2offset.magnitude()),
						 IBK::MSG_WARNING, FUNC_ID);
	}

	// We have 3 equations, but only two unknowns - so we have 3 different options to compute them.
	// Some of them may fail, so we try them all.

	const Vector3D & rhs = v2-offset;

	// compute project of rhs vector to a and b vectors
	IBKMK::Vector3D anorm = a.normalized();
	IBKMK::Vector3D bnorm = b.normalized();
	if (std::fabs(anorm.scalarProduct(bnorm)) < 1e-10) {
//		IBK::IBK_Message("scalar");
		x = rhs.scalarProduct(anorm);
		x /= a.magnitude();
		y = rhs.scalarProduct(bnorm);
		y /= b.magnitude();
	}
	else {
//		IBK::IBK_Message("gleichungssystem");
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
	}

	// check that the point was indeed in the plane
	IBKMK::Vector3D v3 = offset + x*a + y*b;
	IBKMK::Vector3D v3offset = v3 - v;
	if (showWarings && v3offset.magnitude() > tolerance) {
            IBK::IBK_Message(IBK::FormatString("Plane coordinate calculation incorrect: deviation = %1").arg(v3offset.magnitude()), IBK::MSG_WARNING, FUNC_ID);
		return false;
	}
	else
		return true;
}


double lineToPointDistance(const Vector3D & a, const Vector3D & d, const Vector3D & p,
												   double & lineFactor, Vector3D & p2)
{
	// vector from starting point of line to target point
	Vector3D v = p - a;

	// scalar product (projection of v on d) gives scale factor
	lineFactor = v.scalarProduct(d);
	double directionVectorProjection = d.scalarProduct(d); // magnitude squared

	lineFactor /= directionVectorProjection; // normalize line factor

	// compute "lotpunkt"
	p2 = a + lineFactor * d;

	// return distance between lotpunkt and target point
	return (p2-p).magnitude();
}


bool lineShereIntersection(const Vector3D & a, const Vector3D & d, const Vector3D & p, double r,
						   double & lineFactor, Vector3D & lotpoint)
{
	// compute lotpoint and distance between line and sphere center
	double lineFactorToLotPoint;
	double distanceLineToSphereCenter = lineToPointDistance(a, d, p, lineFactorToLotPoint, lotpoint);

	// pass by?
	if (distanceLineToSphereCenter > r)
		return false;

	// solve for intersection with radius
	// extreme cases:
	//   r = b  -> x = 0, intersection with sphere = lotpoint
	//   b = 0  -> x = r, line passes through center of sphere, our sphere intersection point is distance
	//                    'r' closer to the point a
	double x = std::sqrt(r*r - distanceLineToSphereCenter*distanceLineToSphereCenter);
	// and normalize to get distance as fraction of direction vector (line factor)
	x /= d.magnitude();
	// subtract distance to get lineFactor to intersection point with sphere
	lineFactor = lineFactorToLotPoint-x;
	return true;
}


double lineToLineDistance(const Vector3D & a1, const Vector3D & d1,
												  const Vector3D & a2, const Vector3D & d2,
												  double & l1, Vector3D & p1, double & l2)
{
	/// source: http://geomalgorithms.com/a02-_lines.html
	Vector3D v = a1 - a2;

	double d1Scalar = d1.scalarProduct(d1);// always >= 0
	double d1d2Scalar = d1.scalarProduct(d2);
	double d2Scalar = d2.scalarProduct(d2);

	double d1vScalar = d1.scalarProduct(v);// always >= 0
	double d2vScalar = d2.scalarProduct(v);

	double d = d1Scalar*d2Scalar - d1d2Scalar*d1d2Scalar;// always >= 0

	// compute the line parameters of the two closest points
	if (d<1E-4) { // the lines are almost parallel
		l1 = 0.0; // we have to set one factor to determine a point since there are infinite
		l2 = (d1d2Scalar>d2Scalar ? d1vScalar/d1d2Scalar : d2vScalar/d2Scalar);    // use the largest denominator
	}
	else {
		l1 = (d1d2Scalar*d2vScalar - d2Scalar*d1vScalar) / d;
		l2 = (d1Scalar*d2vScalar - d1d2Scalar*d1vScalar) / d;
	}

	p1 = a1 + ( l1 * d1 );					// point 1
	Vector3D p2 = a2 + (l2 * d2 );	// point 2

	// get the difference of the two closest points
	return ( p1 - p2 ).magnitude();   // return the closest distance
}


void pointProjectedOnPlane(const Vector3D & a, const Vector3D & normal, const Vector3D & p, Vector3D & projectedP) {
	// vector from point to plane origin
	projectedP = p-a;
	// scalar product between this vector and plane's normal
	double dist = normal.m_x*projectedP.m_x + normal.m_y*projectedP.m_y + normal.m_z*projectedP.m_z;
	// subtract normal vector's contribution of project P from p
	projectedP = p - dist*normal;
}


void eliminateCollinearPoints(std::vector<IBKMK::Vector3D> & polygon, double epsilon) {
	if (polygon.size()<2)
		return;

	// check for duplicate points in polyline and remove duplicates

	// the algorithm works as follows:
	// - we start at current index 0
	// - we compare the vertex at current index with that of the next vertex
	//    - if both are close enough together, we elminate the current vertex and try again
	//    - otherwise both vertexes are ok, and we advance the current index
	// - algorithm is repeated until we have processed the last point of the polygon
	// Note: when checking the last point of the polygon, we compare it with the first vertex (using modulo operation).
	unsigned int i=0;
	double eps2 = epsilon*epsilon;
	while (polygon.size() > 1 && i < polygon.size()) {
		// distance between current and next point
		IBKMK::Vector3D diff = polygon[i] - polygon[(i+1) % polygon.size()]; // Note: when i = size-1, we take different between last and first element
		if (diff.magnitudeSquared() < eps2)
			polygon.erase(polygon.begin()+i); // remove point and try again
		else
			++i;
	}

	// Now we have only different points. We process the polygon again and remove all vertexes between collinear edges.
	// The algorithm uses the following logic:
	//    - take 3 subsequent vertexes, compute lines from vertex 1-2 and 1-3
	//    - compute projection of line 1 onto 2
	//    - compute projected end point of line 1 in line 2
	//    - if distance between projected point and original vertex 2 is < epsison, remove vertex 2

	i=0;
	while (polygon.size() > 1 && i < polygon.size()) {
		// we check if we can remove the current vertex i
		// take the last and next vertex
		const IBKMK::Vector3D & last = polygon[(i + polygon.size() - 1) % polygon.size()];
		const IBKMK::Vector3D & next = polygon[(i+1) % polygon.size()];
		// compute vertex a and b and normalize
		IBKMK::Vector3D a = next - last; // vector to project on
		IBKMK::Vector3D b = polygon[i] - last; // vector that shall be projected
		double anorm2 = a.magnitudeSquared();
		if (anorm2 < eps2) {
			// next and last vectors are nearly identical, hence we have a "spike" geometry and need to remove
			// both the spike and one of the identical vertexes
			polygon.erase(polygon.begin()+i); // remove point (might be the last)
			if (i < polygon.size())
				polygon.erase(polygon.begin()+i); // not the last point, remove next as well
			else
				polygon.erase(polygon.begin()+i-1); // remove previous point
			continue;
		}

		// compute projection vector
		IBKMK::Vector3D astar = a.scalarProduct(b)/anorm2 * a;
		// get vertex along vector b
		astar += last;
		// compute difference
		IBKMK::Vector3D diff = polygon[i] - astar;
		if (diff.magnitudeSquared() < eps2)
			polygon.erase(polygon.begin()+i); // remove point and try again
		else
			++i;
	}
}


bool linePlaneIntersectionWithNormalCheck(const Vector3D & a, const Vector3D & normal, const Vector3D & p,
		const IBKMK::Vector3D & d, IBKMK::Vector3D & intersectionPoint, double & dist, bool checkNormal)
{
	// plane is given by offset 'a' and normal vector 'normal'.
	// line is given by point 'p' and its line vector 'd'

	// first the normal test

	double d_dot_normal = d.scalarProduct(normal);
	double angle = d_dot_normal/d.magnitude();
	// line parallel to plane? no intersection
	if (angle < 1e-8 && angle > -1e-8)
		return false;

	// Condition 1: same direction of normal vectors?
	if (checkNormal && angle >= 0)
		return false; // no intersection possible

	double t = (a - p).scalarProduct(normal) / d_dot_normal;

	// now determine location on plane
	IBKMK::Vector3D x0 = p + t*d;

	intersectionPoint = x0;
	dist = t;
	return true;
}


bool linePlaneIntersection(const Vector3D & a, const Vector3D & normal, const Vector3D & p,
		const IBKMK::Vector3D & d, IBKMK::Vector3D & intersectionPoint, double & dist)
{
	// plane is given by offset 'a' and normal vector 'normal'.
	// line is given by point 'p' and its line vector 'd'

	// first the normal test

	double d_dot_normal = d.scalarProduct(normal);
	double angle = d_dot_normal/d.magnitude();
	// line parallel to plane? no intersection
	if (angle < 1e-8 && angle > -1e-8)
		return false;

	double t = (a - p).scalarProduct(normal) / d_dot_normal;

	// now determine location on plane
	IBKMK::Vector3D x0 = p + t*d;

	intersectionPoint = x0;
	dist = t;
	return true;
}


void enlargeBoundingBox(const Vector3D & v, Vector3D & minVec, Vector3D & maxVec) {
	minVec.m_x = std::min(minVec.m_x, v.m_x);
	minVec.m_y = std::min(minVec.m_y, v.m_y);
	minVec.m_z = std::min(minVec.m_z, v.m_z);

	maxVec.m_x = std::max(maxVec.m_x, v.m_x);
	maxVec.m_y = std::max(maxVec.m_y, v.m_y);
	maxVec.m_z = std::max(maxVec.m_z, v.m_z);
}


int coplanarPointInPolygon3D(const std::vector<Vector3D> polygon, const IBK::point3D<double> point) {
	IBK_ASSERT(polygon.size() >= 3);
	// Todo: eliminateCollinearPoints
	// use cross product to obtain normal vector
	IBKMK::Vector3D polyNormalVector = (polygon[1] - polygon[0]).crossProduct(polygon[2] - polygon[0]);

	std::vector<Vector2D> polygon2D;
	IBK::point2D<double> point2D;
	if (polyNormalVector.m_z != 0) {
		//normal vector contains a "z" component, i.e. polygon contains x&y components, and can be projected in x-y-plane.
		//other cases respective, point respective
		point2D.m_x = point.m_x;
		point2D.m_y = point.m_y;
		for (Vector3D i : polygon) {
			polygon2D.push_back(Vector2D(i.m_x,i.m_y)); // eliminate z
		}
	} else if (polyNormalVector.m_y != 0) {
		point2D.m_x = point.m_x;
		point2D.m_y = point.m_z;
		for (Vector3D i : polygon) {
			polygon2D.push_back(Vector2D(i.m_x,i.m_z)); // eliminate y
		}
	} else if (polyNormalVector.m_x != 0) {
		point2D.m_x = point.m_z;
		point2D.m_y = point.m_y;
		for (Vector3D i : polygon) {
			polygon2D.push_back(Vector2D(i.m_z,i.m_y)); // eliminate x
		}
	}
	return pointInPolygon(polygon2D, point2D);
}


bool polyIntersect(const std::vector<Vector3D> & vertsAexact, const std::vector<Vector3D> & vertsBexact) {
	IBK_ASSERT(vertsAexact.size() >= 3);
	IBK_ASSERT(vertsBexact.size() >= 3);

	IBK::NearEqual<double> near_equal5(1e-5);
	IBK::NearEqual<double> near_equal1(1e-1);

	const double coordFactor = 1e4;

	std::vector<Vector3D> vertsA;
	std::vector<Vector3D> vertsB;

	// rounding all vertices to achieve near_equal precision
	for (Vector3D i : vertsAexact) {
			vertsA.push_back(Vector3D(std::round(i.m_x*coordFactor)/coordFactor,std::round(i.m_y*coordFactor)/coordFactor,std::round(i.m_z*coordFactor)/coordFactor));
	}
	for (Vector3D i : vertsBexact) {
			vertsB.push_back(Vector3D(std::round(i.m_x*coordFactor)/coordFactor,std::round(i.m_y*coordFactor)/coordFactor,std::round(i.m_z*coordFactor)/coordFactor));
	}



	// get arbitrary base vectors for polygon A and B's planes
	// TODO: error handling in case polygon is malformatted and first 3 points are located on a straight line .. eliminateCollinearPoints?
	// compensating for different sizes of span vectors to have later calculations in the same order of magnitude
	IBKMK::Vector3D vectorA1 = vertsA[1] - vertsA[0];
	vectorA1 = vectorA1 * (10/vectorA1.magnitude());
	IBKMK::Vector3D vectorA2 = vertsA[2] - vertsA[0];
	vectorA2 = vectorA2 * (10/vectorA2.magnitude());

	IBKMK::Vector3D vectorB1 = vertsB[1] - vertsB[0];
	vectorB1 = vectorB1 * (10/vectorB1.magnitude());
	IBKMK::Vector3D vectorB2 = vertsB[2] - vertsB[0];
	vectorB2 = vectorB2 * (10/vectorB2.magnitude());

	// use cross product to obtain normal vectors
	IBKMK::Vector3D normalVectorA = vectorA1.crossProduct(vectorA2);
	IBKMK::Vector3D normalVectorB = vectorB1.crossProduct(vectorB2);

	int offsetA = normalVectorA.scalarProduct(vertsA[0]);
	int offsetB = normalVectorB.scalarProduct(vertsB[0]);

	// check if polygon planes A & B are parallel
	// if crossProduct of normal vectors returns 0-vector then normal vectors are parallel
	// magnitude of normal vector will quickly exceed 1e+2 for small rotations, so 1e-2 check is suited
	if (near_equal1(normalVectorA.crossProduct(normalVectorB).magnitude(), 0)) {
		// planes are parallel
		// ### for parallel cases intersection is not intended.
		// ### otherwise 2D intersection for the coplanar case can be implemented like this:
		/*
		// check if they are coplanar
		//get distance of plane B from plane A by inserting first point of B into HesseNormalForm of plane A (i.e. normal vector)
		double distBFromPlaneA = normalVectorA.scalarProduct(vertsB[0]) - offsetA;
		if (distBFromPlaneA != 0) {
			// planes are parallel but not coplanar, intersection is impossible
			return false;
		} else {
			// planes are parallel and coplanar, 2D intersection needs to be checked.
			// due to the polygons being parallel, we can create a 2d projection of our polygons
			// simply by eliminating one dimension, as long as both the
			// remaining 2 dimensions are not orthogonal to our polygon plane
			std::vector<Vector2D> vertsA2D;
			std::vector<Vector2D> vertsB2D;
			if (normalVectorA.m_z != 0) {
				//normal vector contains a "z" component, i.e. polygons contain x&y components, and can be projected in x-y-plane.
				//other cases respective
				for (unsigned int i = 0, count = vertsA.size(); i<count; ++i ) {
					vertsA2D[i].m_x = vertsA[i].m_x;
					vertsA2D[i].m_y = vertsA[i].m_y;
				}
				for (unsigned int i = 0, count = vertsB.size(); i<count; ++i ) {
					vertsB2D[i].m_x = vertsB[i].m_x;
					vertsB2D[i].m_y = vertsB[i].m_y;
				}
			} else if (normalVectorA.m_y != 0) {
				for (unsigned int i = 0, count = vertsA.size(); i<count; ++i ) {
					vertsA2D[i].m_x = vertsA[i].m_x;
					vertsA2D[i].m_y = vertsA[i].m_z; // eliminate y
				}
				for (unsigned int i = 0, count = vertsB.size(); i<count; ++i ) {
					vertsB2D[i].m_x = vertsB[i].m_x;
					vertsB2D[i].m_y = vertsB[i].m_z;
				}
			} else if (normalVectorA.m_x != 0) {
				for (unsigned int i = 0, count = vertsA.size(); i<count; ++i ) {
					vertsA2D[i].m_x = vertsA[i].m_z; // eliminate x
					vertsA2D[i].m_y = vertsA[i].m_y;
				}
				for (unsigned int i = 0, count = vertsB.size(); i<count; ++i ) {
					vertsB2D[i].m_x = vertsB[i].m_z;
					vertsB2D[i].m_y = vertsB[i].m_y;
				}
			} else {
				// TODO THROW EXCEPTION: malformatted polygon A, normalvector == 0vector, i.e. adjacent polygon edges are in parallel
				return false;
			}
			return polyIntersect2D(vertsA2D, vertsB2D);
		}
		*/
		return false;
	} else {
		// planes are not parallel
		// iterate over all edges in B and find intersection points with plane A, respective for A&B
		IBKMK::Vector3D edgeA;
		IBKMK::Vector3D edgeB;
		IBKMK::Vector3D pointOfIntersection;

		// list of all points of intersection, which is needed later
		std::vector<IBKMK::Vector3D> intersectionPointsOfPolyEdgeAndIntersectionLine;
		double r = 0;

		// first we iterate over polygon edges and test for intersection points with the other polygon's plane
		for (unsigned int i = 0, count = vertsA.size(); i<count; ++i ) {
			edgeA = vertsA[(i+1)%vertsA.size()] - vertsA[i];
			// ensure edge is not in parallel to the other polygon plane, i.e. edge is not orthogonal to normal vector
			if (normalVectorB.scalarProduct(edgeA) != 0) {
				// if vertices inserted into HNF have different signs -> they are on different sides of the plane (i.e. edge-plane intersection exists)
				if (( (normalVectorB.scalarProduct(vertsA[i])-offsetB) * (normalVectorB.scalarProduct(vertsA[(i+1)%vertsA.size()])-offsetB) ) < 0) {
					// calculating point of intersection
					// ...by inserting edgeA into plane B to get factor r, for our edge equation
					// equation: normalVectorB * (vertsA[i] + r * edgeA) == offsetB;
					// division by zero can't occure because we already checked for parallelism
					r = (offsetB - normalVectorB.scalarProduct(vertsA[i])) / normalVectorB.scalarProduct(edgeA);
					// get point of intersection:
					pointOfIntersection = vertsA[i] + r * edgeA;
					intersectionPointsOfPolyEdgeAndIntersectionLine.push_back(pointOfIntersection);
					if (coplanarPointInPolygon3D(vertsB, pointOfIntersection) == 1) {
						return true;
					}
				}
			}
		}
		// edges(B) intersecting plane(A) respective
		for (unsigned int i = 0, count = vertsB.size(); i<count; ++i ) {
			edgeB = vertsB[(i+1)%vertsB.size()] - vertsB[i];
			if (normalVectorA.scalarProduct(edgeB) != 0) {
				if (( (normalVectorA.scalarProduct(vertsB[i])-offsetA) * (normalVectorA.scalarProduct(vertsB[(i+1)%vertsB.size()])-offsetA) ) < 0) {
					r = (offsetA - normalVectorA.scalarProduct(vertsB[i])) / normalVectorA.scalarProduct(edgeB);
					pointOfIntersection = vertsB[i] + r * edgeB;
					intersectionPointsOfPolyEdgeAndIntersectionLine.push_back(pointOfIntersection);
					if (coplanarPointInPolygon3D(vertsA, pointOfIntersection) == 1) {
						return true;
					}
				}
			}
		}

		// We now know the planes intersect, but none of the edges create an intersection point within the other polygon.
		// Next we check for the case of the polygons sharing points or edges

		// directional vector of intersection line:
		IBKMK::Vector3D dirVector = normalVectorA.crossProduct(normalVectorB);

		// support vector of intersection line:
		// vectors in parallel to each plane, orthogonal to intersection and normal vector
		IBKMK::Vector3D vectorParallelPlaneA = dirVector.crossProduct(normalVectorA);
		IBKMK::Vector3D vectorParallelPlaneB = dirVector.crossProduct(normalVectorB);

		// calculate intersection point of each parallel vector and the respective other plane
		double factorVectorBPlaneA = offsetA / normalVectorA.scalarProduct(vectorParallelPlaneB);
		double factorVectorAPlaneB = offsetB / normalVectorB.scalarProduct(vectorParallelPlaneA);

		// linear combination of the two parallel vectors scaled to the intersection points
		// this results in a vector/point that lies on both planes, and therefore functions as support vector
		IBKMK::Vector3D supportVector = (vectorParallelPlaneA * factorVectorAPlaneB) + (vectorParallelPlaneB * factorVectorBPlaneA);

		// find all polygon vertices which lie on the line
		std::map<double, IBKMK::Vector3D> polyPointsOnIntersectionLine = {/*{ 1, IBKMK::Vector3D( 1,  2,  3) }*/};
		double intersectionToPolyvertexFactor;
		Vector3D temporaryVector;

		for (unsigned int i = 0, count = vertsA.size(); i<count; ++i ) {
			//avoid division by zero
			if (dirVector.m_x != 0) {
				intersectionToPolyvertexFactor = (vertsA[i].m_x - supportVector.m_x) / dirVector.m_x;
			} else if (dirVector.m_y != 0) {
				intersectionToPolyvertexFactor = (vertsA[i].m_y - supportVector.m_y) / dirVector.m_y;
			} else /*if (dirVector.m_z != 0)*/ {
				intersectionToPolyvertexFactor = (vertsA[i].m_z - supportVector.m_z) / dirVector.m_z;
			}
			temporaryVector = (supportVector + intersectionToPolyvertexFactor * dirVector);
			if (near_equal5(temporaryVector.m_x, vertsA[i].m_x) && near_equal5(temporaryVector.m_y, vertsA[i].m_y) && near_equal5(temporaryVector.m_z, vertsA[i].m_z)) {
				//using a map avoids duplicates
				polyPointsOnIntersectionLine.insert({intersectionToPolyvertexFactor, vertsA[i]});
			}
		}
		for (unsigned int i = 0, count = vertsB.size(); i<count; ++i ) {
			//avoid division by zero
			if (dirVector.m_x != 0) {
				intersectionToPolyvertexFactor = (vertsB[i].m_x - supportVector.m_x) / dirVector.m_x;
			} else if (dirVector.m_y != 0) {
				intersectionToPolyvertexFactor = (vertsB[i].m_y - supportVector.m_y) / dirVector.m_y;
			} else /*if (dirVector.m_z != 0)*/ {
				intersectionToPolyvertexFactor = (vertsB[i].m_z - supportVector.m_z) / dirVector.m_z;
			}
			temporaryVector = (supportVector + intersectionToPolyvertexFactor * dirVector);
			if (near_equal5(temporaryVector.m_x, vertsB[i].m_x) && near_equal5(temporaryVector.m_y, vertsB[i].m_y) && near_equal5(temporaryVector.m_z, vertsB[i].m_z)) {
				//using a map avoids duplicates
				polyPointsOnIntersectionLine.insert({intersectionToPolyvertexFactor, vertsB[i]});
			}
		}

		//add the points, where poly edges intersect the other polygons plane, to the map
		for (unsigned int i = 0, count = intersectionPointsOfPolyEdgeAndIntersectionLine.size(); i<count; ++i ) {
			//we already know the points lie on the intersection line, but for automatic sorting we need to calculate the intersectionToPolyvertexFactor
			//avoid division by zero
			if (dirVector.m_x != 0) {
				intersectionToPolyvertexFactor = (intersectionPointsOfPolyEdgeAndIntersectionLine[i].m_x - supportVector.m_x) / dirVector.m_x;
			} else if (dirVector.m_y != 0) {
				intersectionToPolyvertexFactor = (intersectionPointsOfPolyEdgeAndIntersectionLine[i].m_y - supportVector.m_y) / dirVector.m_y;
			} else /*if (dirVector.m_z != 0)*/ {
				intersectionToPolyvertexFactor = (intersectionPointsOfPolyEdgeAndIntersectionLine[i].m_z - supportVector.m_z) / dirVector.m_z;
			}
			polyPointsOnIntersectionLine.insert({intersectionToPolyvertexFactor, intersectionPointsOfPolyEdgeAndIntersectionLine[i]});

		}

		// std::map is already ordered by key
		if (polyPointsOnIntersectionLine.size() >=2) {

			// iterate over all center points inbetween poly vertices on intersection lines
			IBKMK::Vector3D centerpoint;

			for (auto it = polyPointsOnIntersectionLine.begin(); it != std::prev(polyPointsOnIntersectionLine.end()); ++it) {
				auto next = std::next(it);
				centerpoint = it->second + (next->second - it->second) * 0.5;

				// test if centerpoint is contained within both polygons
				if ((coplanarPointInPolygon3D(vertsA, centerpoint) == 1) && (coplanarPointInPolygon3D(vertsB, centerpoint) == 1)) {
					return true;
				}
			}
		}
	}
return false;
}

} // namespace IBKMK
