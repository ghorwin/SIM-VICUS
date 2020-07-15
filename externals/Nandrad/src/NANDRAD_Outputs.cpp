/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
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

#include "NANDRAD_Outputs.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>

#include "NANDRAD_Constants.h"

#include <tinyxml.h>

namespace NANDRAD {


bool Outputs::operator!=(const Outputs & other) const {
	if (m_outputDefinitions.size() != other.m_outputDefinitions.size()) return true;
	for (unsigned int i=0; i<m_outputDefinitions.size(); ++i)
		if (m_outputDefinitions[i] != other.m_outputDefinitions[i]) return true;

	if (m_grids.size() != other.m_grids.size()) return true;
	for (unsigned int i=0; i<m_grids.size(); ++i)
		if (m_grids[i] != other.m_grids[i]) return true;

	if (m_binaryFormat != other.m_binaryFormat) return true;
	if (m_timeUnit != other.m_timeUnit) return true;

	return false;
}



} // namespace NANDRAD

