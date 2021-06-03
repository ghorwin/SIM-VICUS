/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#ifndef NANDRAD_WindowShadingH
#define NANDRAD_WindowShadingH

#include <algorithm>

#include <IBK_Parameter.h>
#include <IBK_Exception.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_LinearSplineParameter.h"

namespace NANDRAD {

class ShadingControlModel;


/*!	WindowShading defines a dynamically adjustable shading. */
class WindowShading  {
	NANDRAD_READWRITE_PRIVATE
public:

	/*! Model types supported by the shading model. */
	enum modelType_t {
		MT_Constant,					// Keyword: Constant						'Constant reduction factor.'
		MT_Precomputed,					// Keyword: Precomputed						'Precomputed reduction factor.'
		MT_Controlled,					// Keyword: Controlled						'Reduction factor is computed based on control model'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_ReductionFactor,				// Keyword: ReductionFactor			[---]	'Reduction factor (remaining percentage of solar gains if shading is closed).'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(WindowShading)
	NANDRAD_COMP(WindowShading)

	void checkParameters(const std::vector<ShadingControlModel> &controlModels);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type (NUM_MT disables model). */
	modelType_t							m_modelType = NUM_MT;					// XML:A:required
	/*! Control model used for shading, for model type 'Controlled'. */
	unsigned int						m_controlModelId = INVALID_ID;			// XML:A
	/*! List of constant parameters.*/
	IBK::Parameter						m_para[NUM_P];							// XML:E

	/*! Precomputed shading factor as time series.
		Interpretation and definition is done exactly like climatic data. Cyclic spline data must not exceed one year.
		Start time shift is applied when evaluating value for given simulation time.
	*/
	LinearSplineParameter				m_precomputedReductionFactor;			// XML:E

}; // WindowShading


} // namespace NANDRAD

#endif // NANDRAD_WindowShadingH
