/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef VICUS_PlaneTriangulationDataH
#define VICUS_PlaneTriangulationDataH

#include <IBKMK_Vector3D.h>
#include <IBKMK_Triangulation.h>

namespace VICUS {

/*! Class PlaneTriangulationData holds data needed to draw a triangulated plane.
	This is basically a vector of vertexes and a vector of triangles.
*/
class PlaneTriangulationData {
public:

	PlaneTriangulationData() = default;

	/*! Convenience constructor, creates an object for a triangle. */
	PlaneTriangulationData(const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) {
		m_vertexes.push_back(a);
		m_vertexes.push_back(b);
		m_vertexes.push_back(c);
		m_normal = (b-a).crossProduct(c-a).normalized();
		m_triangles.push_back(IBKMK::Triangulation::triangle_t(0,1,2));
	}
	// *** PUBLIC MEMBER FUNCTIONS ***

	void clear() {
		m_triangles.clear();
		m_vertexes.clear();
	}

	/*! Contains the vertex indexes for each triangle that the plane is composed of. */
	std::vector<IBKMK::Triangulation::triangle_t>		m_triangles;

	/*! The vertexes used by the triangles. */
	std::vector<IBKMK::Vector3D>						m_vertexes;

	/*! The normal vector. */
	IBKMK::Vector3D										m_normal;
};

} // namespace VICUS

#endif // VICUS_PlaneTriangulationDataH
