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

#ifndef IBKMK_TriangulationH
#define IBKMK_TriangulationH

#include <IBK_point.h>
#include <IBK_assert.h>

namespace IBKMK {

/*! Performs triangulation.
	This class wraps the internal triangulation library so that users do not need to know
	the details of the underlying library's API.
*/
class Triangulation {
public:
	/*! Simple storage member to hold vertex indexes of a single triangle. */
	struct triangle_t {
		triangle_t() {}
		triangle_t(unsigned int n1, unsigned int n2, unsigned int n3) :
			i1(n1), i2(n2), i3(n3)
		{}

		/*! Returns true, if triangle is invalid, i.e. contains invalid ID, or twice the same ID. */
		bool isDegenerated() const {
			if (i1 == (unsigned int)-1 || i1 == i2 || i1 == i3) return true;
			if (i2 == (unsigned int)-1 || i2 == i1 || i2 == i3) return true;
			if (i3 == (unsigned int)-1 || i3 == i1 || i3 == i2) return true;
			return false;
		}

		unsigned int i1=0, i2=0, i3=0;
	};

	/*! Set points to triangulate.
		No duplicate points (within tolerance allowed!)
		Also, edges must mark outer and inner boundaries of surface.
	*/
	bool setPoints(const std::vector<IBK::point2D<double> > & points,
				   const std::vector<std::pair<unsigned int, unsigned int> > & edges);

	/*! Tolerance criterion - points within this distances are
		takes and "same".
	*/
	double	m_tolerance;

	/*! Contains the generated triangles after triangulation has completed. */
	std::vector<triangle_t>		m_triangles;
};

} // namespace IBKMK

#endif // IBKMK_TriangulationH
