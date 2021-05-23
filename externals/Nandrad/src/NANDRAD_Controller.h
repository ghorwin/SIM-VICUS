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

#ifndef NANDRAD_ControllerH
#define NANDRAD_ControllerH

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

/*! Data structure to define a controller.
	Controller objects are explicitely defined in the project when they control more than one object/model. They
	can then be referenced by the models that use them via their ID. Most models, however, will contain own
	parametrization of control strategies that are meaningful to the respective model.

	TODO : remove if not needed
*/
class Controller {
public:
	/*! Different model variants. */
	enum ModelType {
		MT_DigitalDirect,		// Keyword: DigitalDirect			'DigitalDirect'
		MT_DigitalHysteresis,	// Keyword: DigitalHysteresis		'DigitalHysteresis'
		MT_PController,			// Keyword: PController				'PController'
		MT_PIController,		// Keyword: PIController			'PIController'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_Kp,					// Keyword: Kp						'Kp-parameter'
		P_Ki,					// Keyword: Ki						'Ki-parameter'
		P_Kd,					// Keyword: Kd						'Kd-parameter'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(Controller)

	/*! Checks for valid and required parameters (value ranges).
	*/
	void checkParameters();

	// *** PUBLIC MEMBER VARIABLES ***

	IDType				m_id = INVALID_ID;							// XML:A:required

	/* Controller type. */
	ModelType			m_modelType = NUM_MT;						// XML:A:required

	/*! Controller parameters. */
	IBK::Parameter		m_para[NUM_P];								// XML:E
};


}  // namespace NANDRAD

#endif // NANDRAD_ControllerH
