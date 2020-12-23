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

#ifndef IBKMK_3DCalculationsH
#define IBKMK_3DCalculationsH

#include "IBKMK_Vector3D.h"

namespace IBKMK {


/*! Computes the distance between a line (defined through offset point a, and directional vector d) and a point p.
	\return Returns the shortest distance between line and point. Factor lineFactor contains the scale factor for
			the line equation and p2 contains the closest point on the line (p2 = a + lineFactor*d).
*/
double lineToPointDistance(const IBKMK::Vector3D & a, const IBKMK::Vector3D & d, const IBKMK::Vector3D & p,
						   double & lineFactor, IBKMK::Vector3D & p2);

/*! Computes the shortest distance between two lines.
	\return Returns the shortest distance between line and line. Factor l1 contains the scale factor for
		the line equation 1 with p1 as the closest point (p1 = a1 +l1*d1). l2 is the line equation factor for line 2
		and can be used to check if the projection of the closest point is inside the line 2.
*/
double lineToLineDistance(const IBKMK::Vector3D & a1, const IBKMK::Vector3D & d1,
						  const IBKMK::Vector3D & a2, const IBKMK::Vector3D & d2,
						  double & l1, IBKMK::Vector3D & p1, double & l2);

} // namespace IBKMK

#endif // IBKMK_3DCalculationsH
