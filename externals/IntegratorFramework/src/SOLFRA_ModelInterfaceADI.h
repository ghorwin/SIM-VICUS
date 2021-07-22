/*	Solver Control Framework
	Copyright (C) 2010  Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ModelInterfaceADIH
#define ModelInterfaceADIH

#include "SOLFRA_ModelInterface.h"

namespace SOLFRA {

/*!	\brief Declaration for class ModelInterfaceADI

	The class ModelInterfaceADI defines the interface functions for ODE model implementation and
	ADI model capabilities. If a model interface is implemented in a derived class, the framework
	with the available integrators and core integration loop can be used.
	\sa SolverControlFramework
*/
class ModelInterfaceADI : public ModelInterface {
public:
	/*! Enumeration value to specify active flux direction. */
	enum FluxDirection {
		FD_X,
		FD_Y,
		FD_Z,
		FD_ALL
	};

	/*! Sets the current flux direction to consider in calculation of divergences.
		For example, if flux direction is set to FD_X, only the x-flux components
		are used to calculate the divergences, all other flux components are ignored.
		To reset the model to default (all flux directions), call this function with FD_ALL
		as argument.
		\note Setting the active flux direction will not invalidate the solver state (time and y values).
				This means, the a call to setActiveFluxDirection() may be directly followed
				by a call to ydot(). Therefore, the model has to ensure, that the ydot() function
				is implemented minimally and only covers the differences between changing flux
				directions.
	*/
	virtual void setActiveFluxDirection(FluxDirection fluxDir) = 0;
	/*! Returns current active flux direction.
	*/
	virtual FluxDirection activeFluxDirection() const = 0;

	/*! Returns x-index vector sorted for ADI calculation.
		\code
		// convert from x-numbering index to index in vector with solution variables
		unsigned int valueIdx = indicesX[xBasedIndex];
		\endcode
	*/
	virtual unsigned int *indicesX() = 0;
	/*! Returns y-index vector sorted for ADI calculation.
		\sa indicesY()
	*/
	virtual unsigned int *indicesY() = 0;
	/*! Returns z-index vector sorted for ADI calculation.
		\sa indicesX()
	*/
	virtual unsigned int *indicesZ() = 0;

	/*! Retrieves element-based size of system.	*/
	virtual unsigned int nElements() const = 0;

	/*! Retrieves number of equations per element (= size of one block). */
	virtual unsigned int nEquations() const = 0;

};

} // namespace SOLFRA

#endif // ModelInterfaceADIH
