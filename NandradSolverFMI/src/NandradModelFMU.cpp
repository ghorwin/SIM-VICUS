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


NandradModelFMU::NandradModelFMU() :
	m_outputBufferCleared(false)
{
}


// *** FMU Related Functions ***

void NandradModelFMU::setReal(int varID, double value) {
	FUNCID(NandradModelFMU::setRealParameter);

}


void NandradModelFMU::setInteger(int varID, int value) {
	FUNCID(NandradModelFMU::setInteger);
}


void NandradModelFMU::setString(int varID, ConstString  value) {
	FUNCID(NandradModelFMU::setString);
}


void NandradModelFMU::setBoolean(int varID, bool value) {
	FUNCID(NandradModelFMU::setBoolean);
}



void NandradModelFMU::getReal(int varID, double & value) {
	FUNCID(NandradModelFMU::getReal);
}


void NandradModelFMU::getInteger(int varID, int & value) {
	FUNCID(NandradModelFMU::getInteger);
}


void NandradModelFMU::getString(int varID, ConstString & value) {
	FUNCID(NandradModelFMU::getString);
}


void NandradModelFMU::getBoolean(int varID, bool & value) {
	FUNCID(NandradModelFMU::getBoolean);
}


void NandradModelFMU::startCommunicationInterval(double tStart) {
	FUNCID(NandradModelFMU::startCommunicationInterval);
}


void NandradModelFMU::completeCommunicationInterval() {
	FUNCID(NandradModelFMU::completeCommunicationInterval);
}


void NandradModelFMU::pushOutputOnBuffer(double t_out, const double * y_out) {
	FUNCID(NandradModelFMU::pushOutputOnBuffer);
}


void NandradModelFMU::clearOutputBuffer() {
	FUNCID(NandradModelFMU::clearOutputBuffer);
}


void NandradModelFMU::resetOutputBuffer(double t_reset) {
	FUNCID(NandradModelFMU::resetOutputBuffer);
}


