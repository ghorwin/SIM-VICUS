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

#include "IBKMK_Triangulation.h"

#include <IBK_messages.h>

#include "CDT/CDT.h"

namespace IBKMK {

bool Triangulation::setPoints(const std::vector<IBK::point2D<double> > & points,
							  const std::vector<std::pair<unsigned int, unsigned int> > & edges)
{
//	FUNCID(Triangulation::setPoints);

	CDT::Triangulation<double> cdt(CDT::FindingClosestPoint::ClosestRandom); // Note: we don't want to use boost

	IBK_ASSERT(sizeof(CDT::V2d<double>) == sizeof(IBK::point2D<double>));
	// since IBK::point2D<double> and CDT::V2d<double> are internally the same, we can just re-interpret our original
	// vector as vertex vector
	const std::vector<CDT::V2d<double> > * vertices = reinterpret_cast<	const std::vector<CDT::V2d<double> > * >(&points);

	std::vector<CDT::Edge> edgeVec;
	edgeVec.reserve(edges.size());
	for (const std::pair<unsigned int, unsigned int> & p : edges) {
		edgeVec.push_back( CDT::Edge(p.first, p.second) );
	}

	cdt.insertVertices(*vertices);
	cdt.insertEdges(edgeVec);
	cdt.eraseOuterTrianglesAndHoles();

	// now transfer the triangle

	m_triangles.resize(cdt.triangles.size());
	for (unsigned int i=0; i<cdt.triangles.size(); ++i) {
		const CDT::VerticesArr3 & t = cdt.triangles[i].vertices;
		m_triangles[i] = triangle_t((unsigned int)t[0], (unsigned int)t[1], (unsigned int)t[2]);
	}

	return true;
}


} // namespace IBKMK

