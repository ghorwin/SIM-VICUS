/*	NANDRAD Solver Framework and Model Implementation.

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

#ifndef NM_AbstractTimeDependencyH
#define NM_AbstractTimeDependencyH

namespace NANDRAD_MODEL {

/*!	This abstract class interface declares functions needed for time-dependent models.
	Child classes must implement setTime() and optionally stepCompleted().

	When implementing a time-dependent model, you need to inherit both from AbstractModel and
	AbstractTimeDependency. If the model is also state-dependent, you
	also need to inherit from AbstractStateDependency.
*/
class AbstractTimeDependency {
public:

	/*! Dummy destructor (for v-table placement). */
	virtual ~AbstractTimeDependency();

	/*! Main state-changing function.
		This function sets the new time point state. Must be implemented in derived models.
		\param t Simulation time in [s] (solver time).
		\return Returns 0 when calculation was successful, 1 when a recoverable error has been detected,
			2 when something is badly wrong
	*/
	virtual int setTime(double t) = 0;

	/*! Informs the model that a step was successfully completed.
		The time point passed to the function correspond to the current state in the integrator object.
		This function can be used to write restart info, or adjust the state of the model discretely
		between integration steps.
		Default implementation does nothing.
		\param t Simulation time in [s].
	*/
	virtual void stepCompleted(double t) { (void)t; }
};


} // namespace NANDRAD_MODEL

#endif // NM_AbstractTimeDependencyH
