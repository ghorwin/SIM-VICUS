/*	IBK Math Kernel Library
		Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

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

#include "IBKMK_Vector3D.h"

namespace IBKMK {


/* Solves equation system with Cramer's rule:
	 a x + c y = e
	 b x + d y = f
*/
static bool solve(double a, double b, double c,  double d,  double e,  double f, double & x, double & y) {
	double det = a*d - b*c;
	// Prevent division by very small numbers
	if (std::fabs(det) < 1e-6)
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
bool planeCoordinates(const Vector3D & offset, const Vector3D & a, const Vector3D & b,
							 const Vector3D & v, double & x, double & y, double tolerance)
{
	// TODO : first project vector v onto plane (using normal vector) and check distance between v and vOnPlane
	//        if larger than tolerance, bail out
	//        otherwise perform point-coordinate calculation with projected point

	// We have 3 equations, but only two unknowns - so we have 3 different options to compute them.
	// Some of them may fail, so we try them all.

	const Vector3D & rhs = v-offset;
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
	Vector3D v2 = offset + x*a + y*b;
	v2 -= v;
	if (v2.magnitude() > tolerance)
		return false;
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


// Eleminate one coolinear point. If a point is erased return true.
static bool eleminateColinearPtsHelper(std::vector<IBKMK::Vector3D> &polyline) {

	if(polyline.size()<=2)
		return false;

	const double eps = 1e-4;
	unsigned int polySize = (unsigned int)polyline.size();

	for(unsigned int idx=0; idx<polySize; ++idx){
		unsigned int idx0 = idx-1;
		if(idx==0)
			idx0 = polySize-1;

		IBKMK::Vector3D a = polyline.at(idx0) - polyline.at(idx);
		IBKMK::Vector3D b = polyline.at((idx+1) % polySize) - polyline.at(idx);
		a.normalize();
		b.normalize();

		double cosAngle = a.scalarProduct(b);


		if(cosAngle < -1+eps || cosAngle > 1-eps){
			polyline.erase(polyline.begin()+idx);
			return true;
		}
	}
	return false;
}


void eleminateColinearPoints(std::vector<IBKMK::Vector3D> & polygon) {
	if(polygon.size()<2)
		return;
	//check for duplicate points in polyline and remove duplicates
	for (int i=(int)polygon.size()-1; i>=0; --i) {
		if(polygon.size()<2)
			return;
		size_t j=(size_t)i-1;
		if(i==0)
			j=polygon.size()-1;
		if((polygon[(size_t)i]-polygon[j]).magnitude()<0.001)
			polygon.erase(polygon.begin()+i);
	}

	bool tryAgain =true;
	while (tryAgain)
		tryAgain = eleminateColinearPtsHelper(polygon);
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

	// Condition 1: same direction of normal vectors?
	if (angle >= 0)
		return false; // no intersection possible

	double t = (a - p).scalarProduct(normal) / d_dot_normal;

	// now determine location on plane
	IBKMK::Vector3D x0 = p + t*d;

	intersectionPoint = x0;
	dist = t;
	return true;
}


} // namespace IBKMK

