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


#ifndef NandradModelFMUH
#define NandradModelFMUH

#include <SOLFRA_FMUModelInterface.h>

#include <IBK_Unit.h>

#include "NM_NandradModel.h"


/*! This class extends NANDRAD Model by FMU related functionality.
*/
class NandradModelFMU : public NANDRAD_MODEL::NandradModel, public SOLFRA::FMUModelInterface {
public:
	// *** FMU Related Functions ***

	/*! Sets a new input parameter of type double. */
	virtual void setReal(int varID, double value) override;
	/*! Sets a new input parameter of type int. */
	virtual void setInteger(int varID, int value) override;
	/*! Sets a new input parameter of type string. */
	virtual void setString(int varID, ConstString  value) override;
	/*! Sets a new input parameter of type bool. */
	virtual void setBoolean(int varID, bool value) override;

	/*! Retrieves an output parameter of type double. */
	virtual void getReal(int varID, double & value) override;
	/*! Retrieves an output parameter of type int. */
	virtual void getInteger(int varID, int & value) override;
	/*! Retrieves an output parameter of type string. */
	virtual void getString(int varID, ConstString & value) override;
	/*! Retrieves an output parameter of type bool. */
	virtual void getBoolean(int varID, bool & value) override;

	/*! This function is called by the master/control system whenever a communication
		interval is started or restarted.
		The model should implement all functionality related to resetting temporary
		outputs made already in previous iterations over this interval, that means
		clear all outputs past the given tStart value.

		For example, that store output data in memory this function should reset the index
		for the current output. Subsequently new outputs may overwrite existing outputs in memory.

		In solvers with temporary output files these files must be recreated/reset.
	*/
	virtual void startCommunicationInterval(double tStart, bool noSetFMUStatePriorToCurrentPoint) override;

	/*! This function is called by the master/control system whenever a communication
		interval has been completed.
	*/
	virtual void completeCommunicationInterval() override;


	// *** other functions ***

	/*! Called right after init, adjusts constants in Output handler to avoid output flushing in
		the middle of a communication interval.
	*/
	void disableDefaultOutputFlushing();

}; // class NandradModelFMU


#endif // NandradModelFMU
