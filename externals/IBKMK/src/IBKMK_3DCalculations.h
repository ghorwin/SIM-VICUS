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

#ifndef IBKMK_3DCalculationsH
#define IBKMK_3DCalculationsH

#include "IBKMK_Vector3D.h"
#include <IBK_physics.h>

namespace IBKMK {

/*! Computes the coordinates x, y of a point 'p' in a plane spanned by vectors a and b from a point 'offset', where rhs = p-offset.
	The computed plane coordinates are stored in variables x and y (the factors for vectors a and b, respectively).
	If no solution could be found (only possible if a and b are collinear or one of the vectors has length 0?),
	the function returns false.
	Note: when the point p is not in the plane, this function will still get a valid result.
	Optional argument tolerance defines the allowed distance (magnitude of distance vector) of a point from
	the projected point on the plane. This can be used to correct rounding errors. If the distance is larger than
	the given tolerance, the function returns false (rounding error too large).
*/
bool planeCoordinates(const Vector3D & offset, const Vector3D & a, const Vector3D & b,
					  const Vector3D & v, double & x, double & y,
					  double tolerance = 1e-4, bool showWarings = false);


/*! Computes the distance between a line (defined through offset point a, and directional vector d) and a point p.
	\return Returns the shortest distance between line and point. Factor lineFactor contains the scale factor for
			the line equation and p2 contains the closest point on the line (p2 = a + lineFactor*d).
*/
double lineToPointDistance(const Vector3D & a, const Vector3D & d, const Vector3D & p,
						   double & lineFactor, Vector3D & p2);

/*! Computes the shortest distance between two lines.
	\return Returns the shortest distance between line and line. Factor l1 contains the scale factor for
		the line equation 1 with p1 as the closest point (p1 = a1 +l1*d1). l2 is the line equation factor for line 2
		and can be used to check if the projection of the closest point is inside the line 2.
*/
double lineToLineDistance(const Vector3D & a1, const Vector3D & d1,
						  const Vector3D & a2, const Vector3D & d2,
						  double & l1, Vector3D & p1, double & l2);

/*! Calculates intersection with a sphere, given by center point 'p' and radius 'r'.
	Returns true if line intersects sphere. Returns 'lineFactor' from point 'a' to sphere intersection point.
	Also returns lotpoint (closest point on line to sphere center).
*/
bool lineShereIntersection(const Vector3D & a, const Vector3D & d, const Vector3D & p, double r,
						   double & lineFactor, Vector3D & lotpoint);

/*! Calculates intersection of a line with a plane.
	Plane is given by offset 'a' and normal vector 'normal'.
	Line is given by point 'p' and its line vector 'd'.
	\return Returns false, if either line is parallel to plane (normal vector and line vector are perpendicular),
		or if normal vector and line vector point into the same direction. Otherwise returns true and the intersection
		point in 'intersectionPoint' and the line factor 'dist'.
*/
bool linePlaneIntersectionWithNormalCheck(const Vector3D & a, const Vector3D & normal, const Vector3D & p,
						   const IBKMK::Vector3D & lineVector, IBKMK::Vector3D & intersectionPoint, double & dist, bool checkNormal = true);

/*! Calculates intersection of a line with a plane.
	Plane is given by offset 'a' and normal vector 'normal'.
	Line is given by point 'p' and its line vector 'd'.
	\return Returns false if line is parallel to plane (normal vector and line vector are perpendicular).
		Otherwise returns true and the intersection point in 'intersectionPoint' and the line factor 'dist'.
*/
bool linePlaneIntersection(const Vector3D & a, const Vector3D & normal, const Vector3D & p,
						   const IBKMK::Vector3D & lineVector, IBKMK::Vector3D & intersectionPoint, double & dist);

/*! This function computes the projection of a point p in a plane (given by offset 'a' and 'normal' vector).
	Returns projected point coordinates.
*/
void pointProjectedOnPlane(const Vector3D & a, const Vector3D & normal,
						  const Vector3D & p, Vector3D & projectedP);

/*! Eleminates colinear points in a polygon. */
void eliminateCollinearPoints(std::vector<IBKMK::Vector3D> & polygon, double epsilon = 1e-5);

/*! Returns the inner Angle between two Vectors of a Polygon in Degree (0..360). */
inline double angleBetweenVectorsDeg ( const IBKMK::Vector3D &v1, const IBKMK::Vector3D &v2) {
	return std::acos( v1.scalarProduct(v2) / sqrt(v1.magnitude() * v2.magnitude() ) ) / IBK::DEG2RAD;
}

/*! Takes the vector v and enlarges the current bounding box defined through 'minVec' and 'maxVec'. */
void enlargeBoundingBox(const IBKMK::Vector3D & v, IBKMK::Vector3D & minVec, IBKMK::Vector3D & maxVec);

/*! Transforms 3D Polygon to 2D by eliminating one dimension and applies 2D Point in Polygon.
	It's assumed the point is already coplanar. This should previously be tested.
	Point in Polygon function. Result:
	-1 point not in polyline
	0 point on polyline
	1 point in polyline
*/
int coplanarPointInPolygon3D(const std::vector<Vector3D> poly, const IBK::point3D<double> point);

/*! Determines if intersection occurs between polygon and other polygon.
	Touching is not counted as intersecting.
	Returns true in case of intersection.
	Algorithm design:
		 * first we test if planes actually intersect
		 * if so, we have 2 consecutive concepts for detecting intersection:
		 *
		 *	 we iterate over all edges of both polygons and calculate the intersection points with the respective other polygon plane
		 *   if these intersection points are contained within the other polygon, an intersection is detected (true)
		 *
		 *   then we calculate the intersection line between the two planes, and list all polygon vertices which lie on the line
		 *   if there are >=2 (after duplicate elimination) we iterate over the center-points between each neighboring vertices on the line
		 *   if any of the center points is contained within both polygons, an intersection is detected (true)
	This 2-step algorithm ensures we won't miss any edge cases, e.g. polygons sharing intersection points or lines
 */
bool polyIntersect(const std::vector<IBKMK::Vector3D> & vertsA, const std::vector<IBKMK::Vector3D> & vertsB);

} // namespace IBKMK

#endif // IBKMK_3DCalculationsH