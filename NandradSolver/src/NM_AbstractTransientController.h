/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_AbstractTransientControllerH
#define NM_AbstractTransientControllerH

#include "NM_AbstractController.h"

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract controller with integral part.
	This class essentially adds two functions that can are used by time integrators to add time integration
	functionality.
*/
class AbstractTransientController: public AbstractController {
public:
	/*! Sets the new controller error integral so that it can be used in derived classes control value
		calculation functions.
	*/
	void setY(const double *y) { m_errorValueIntegral = y[0]; }

	/*! The error value is the time-derivative of the integral value. */
	void ydot(double *ydot) { ydot[0] = m_errorValue; }

	/*! Cached error value integral. */
	double				m_errorValueIntegral;
};

} // namespace NANDRAD_MODEL

#endif // NM_AbstractTransientControllerH
