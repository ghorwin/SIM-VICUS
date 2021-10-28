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

#include "NANDRAD_HeatLoadSummationModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void HeatLoadSummationModel::checkParameters() {
	FUNCID(HeatLoadSummationModel::checkParameters);

	// m_useZoneCoolingLoad only allows value 'true' or 'false'
	// all models require an object list with indication of construction that this model applies to
	// we enforce an object list for construction instances at the moment
	if (m_objectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ObjectList' parameter."), FUNC_ID);
}


} // namespace NANDRAD

