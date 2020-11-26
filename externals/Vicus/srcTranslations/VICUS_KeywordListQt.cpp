/*	The NANDRAD data model library.

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

#include "VICUS_KeywordListQt.h"

namespace VICUS {
KeywordListQt::KeywordListQt() {

	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Building");
	tr("Mixer");
	tr("Source");
	tr("Triangle");
	tr("Rectangle");
	tr("Polygon");
	tr("All");
	tr("Grid is visible");
}


QString KeywordListQt::Description( const std::string & category, unsigned int keywordId) { 

	std::string description = KeywordList::Description(category.c_str(), keywordId);
	return tr(description.c_str());

}

} // namespace
