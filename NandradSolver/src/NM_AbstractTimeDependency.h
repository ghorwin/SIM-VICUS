/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef AbstractTimeDependencyH
#define AbstractTimeDependencyH

namespace NANDRAD_MODEL {

/*!	\brief Declaration for class AbstractTimeDependency
	\author Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>
	This class handles time-dependent models.
	Child classes must implement timeChanged() and
	AbstractStateDependency::referenceType(). 
	
	A time dependent model object integrates the
	model and the state dependency properties. Therefore, it needs to inherit both from
	AbstractModel and AbstractTimeDependency. If the model is also state-dependent, you 
	should also inherit from AbstractStateDependency.
*/
class AbstractTimeDependency {
public:

	/*! Main state-changing function.
		This function sets the new time point state and calls timeChanged()
		whenever the time point has changed.
		\param t Simulation time in [s].
		\return Returns 0 when calculation was successful, 1 when a recoverable error has been detected, 2 when something is badly wrong
		*/
	virtual int setTime(double t) = 0;

	/*! Informs the model that a step was successfully completed.
		The time point and value vector passed to the function correspond to
		the current state in the integrator object.
		This function can be used to write restart info.
		Default implementation does nothing.
		\param t Simulation time in [s].
	*/
	virtual void stepCompleted(double t) { (void)t; }

private:

	friend class NandradModelImpl;
};


} // namespace NANDRAD_MODEL

#endif // AbstractTimeDependencyH
