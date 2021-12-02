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

#include "NandradModelFMU.h"
#include "NM_FMIInputOutput.h"
#include "NM_OutputHandler.h"

#include <IBK_assert.h>


// *** FMU Related Functions ***

void NandradModelFMU::setReal(int varID, double value) {
	FUNCID(NandradModelFMU::setRealParameter);

	// For now, we ignore call to setRealParameter() *before* initialization was done
	if (m_fmiInputOutput == nullptr)
		return;

	try {
		m_fmiInputOutput->setFMIInputValue((unsigned int) varID, value);
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error setting input value for quantity with FMI id %1!")
							 .arg(varID), FUNC_ID);
	}
}


void NandradModelFMU::setInteger(int /*varID*/, int /*value*/) {
//	FUNCID(NandradModelFMU::setInteger);
}


void NandradModelFMU::setString(int /*varID*/, ConstString  /*value*/) {
//	FUNCID(NandradModelFMU::setString);
}


void NandradModelFMU::setBoolean(int /*varID*/, bool /*value*/) {
//	FUNCID(NandradModelFMU::setBoolean);
}



void NandradModelFMU::getReal(int varID, double & value) {
	FUNCID(NandradModelFMU::getReal);
	IBK_ASSERT(m_fmiInputOutput != nullptr);
	try {
		m_fmiInputOutput->getFMIOutputValue((unsigned int) varID, value);
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error retrieving output value for quantity with FMI id %1!")
							 .arg(varID), FUNC_ID);
	}
}


void NandradModelFMU::getInteger(int varID, int & /*value*/) {
	FUNCID(NandradModelFMU::getInteger);
	throw IBK::Exception(IBK::FormatString("No such integer variable with value reference %1.").arg(varID), FUNC_ID);
}


void NandradModelFMU::getString(int varID, ConstString & /*value*/) {
	FUNCID(NandradModelFMU::getString);
	throw IBK::Exception(IBK::FormatString("No such string variable with value reference %1.").arg(varID), FUNC_ID);
}


void NandradModelFMU::getBoolean(int varID, bool & /*value*/) {
	FUNCID(NandradModelFMU::getBoolean);
	throw IBK::Exception(IBK::FormatString("No such boolean variable with value reference %1.").arg(varID), FUNC_ID);
}


void NandradModelFMU::startCommunicationInterval(double tStart, bool noSetFMUStatePriorToCurrentPoint) {
//	FUNCID(NandradModelFMU::startCommunicationInterval);

	// Note: if we use an iterating master algorithm, this function may be called several times for the same tStart value
	//       and same model state.

	// if we start the simulation from begin, we write initial outputs
	// we pass t0 and y0 for the initial model evaluation within writeOutputs()
	if (tStart == t0()) {
		setTime(t0());
		setY(y0());
		ydot(nullptr);
		writeOutputs( t0(), y0());
	}

	// if the last interval was completed and we will not jump back in time again, we can flush all already collected outputs
	if (noSetFMUStatePriorToCurrentPoint)
		m_outputHandler->flushCache();
}


void NandradModelFMU::completeCommunicationInterval() {
	//	FUNCID(NandradModelFMU::completeCommunicationInterval);
}


void NandradModelFMU::disableDefaultOutputFlushing() {
	// deactivate automatic output writing in FMI mode
	m_outputHandler->m_outputCacheLimit = std::numeric_limits<unsigned int>::max();
	m_outputHandler->m_realTimeOutputDelay = std::numeric_limits<double>::max();
}



