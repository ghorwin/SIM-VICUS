/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NANDRAD_WindowShadingH
#define NANDRAD_WindowShadingH

#include <algorithm>

#include <IBK_Parameter.h>
#include <IBK_Exception.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {


/*!	WindowShading defines a dynamically adjustable shading. */
class WindowShading  {
	NANDRAD_READWRITE_PRIVATE
public:

		/*! Model types supported by the window model. */
	enum modelType_t {
		MT_Standard,					// Keyword: Standard						'Standard reduction factor.'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_ReductionFactor,				// Keyword: ReductionFactor		[W/m2K]		'Reduction factor (remaining percentage of solar gains if shading is closed).'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(WindowShading)
	NANDRAD_COMP(WindowShading)

	void checkParameters();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type (NUM_MT disables model). */
	modelType_t							m_modelType = NUM_MT;					// XML:A:required
	/*! Control model used for shading. */
	unsigned int						m_controlModelID = INVALID_ID;			// XML:E
	/*! List of constant parameters.*/
	IBK::Parameter						m_para[NUM_P];							// XML:E


}; // WindowShading


inline bool WindowShading::operator!=(const WindowShading & other) const {
	if (m_controlModelID != other.m_controlModelID) return true;
	if (m_modelType != other.m_modelType) return true;
	for (unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	return false;
}


inline void WindowShading::checkParameters() {
	FUNCID(WindowShading::checkParameters);


	if (m_modelType == NUM_MT)
		return;

	switch (m_modelType) {
		case NUM_MT :
			return; // only check if enabled

		case MT_Standard :
			if (m_controlModelID == INVALID_ID)
				throw IBK::Exception("Shading model requires reference to shading control model (tag 'ControlModelID')", FUNC_ID);
			m_para[P_ReductionFactor].checkedValue("%", "%", 0, true, 1, true,
								"Reduction factor be between 0 and 1.");
	}

}


} // namespace NANDRAD

#endif // NANDRAD_WindowShadingH
