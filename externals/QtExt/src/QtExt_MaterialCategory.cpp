/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#include <QtCore>

#include "QtExt_MaterialCategory.h"

namespace QtExt{

MaterialCategory::MaterialCategory() :
	idx(NUM_CATEGORIES)
{
}

MaterialCategory::MaterialCategory(category_t cat) {
	if (static_cast<unsigned int>(cat) >= NUM_CATEGORIES)
		idx = NUM_CATEGORIES;
	else
		idx = cat;
}

QString MaterialCategory::name(category_t idx) {
	switch (idx) {
		case COATING:			return tr("Coating", "MaterialCategory");
		case PLASTER:			return tr("Plaster and mortar", "MaterialCategory");
		case BRICK:				return tr("Building brick", "MaterialCategory");
		case NATURAL_STONES:	return tr("Natural stones", "MaterialCategory");
		case CONCRETE:			return tr("Concrete containing building materials", "MaterialCategory");
		case INSULATION:		return tr("Insulation materials", "MaterialCategory");
		case BUILDING_BOARD:	return tr("Building boards", "MaterialCategory");
		case TIMBER:			return tr("Timber", "MaterialCategory");
		case NATURAL_MATERIALS:	return tr("Natural materials", "MaterialCategory");
		case SOIL:				return tr("Soil", "MaterialCategory");
		case CLADDING:			return tr("Cladding panels and ceramic tiles", "MaterialCategory");
		case FOIL:				return tr("Foils and waterproofing products", "MaterialCategory");
		case MISCELLANEOUS:		return tr("Miscellaneous", "MaterialCategory");
		default: return tr("Unknown", "MaterialCategory");
	}
}

int MaterialCategory::getIndex(const QString& str) {
	for( size_t i=0 ; i < static_cast<size_t>(NUM_CATEGORIES); ++i)
		if( str == name(static_cast<MaterialCategory::category_t>(i)))
			return (int)i;
	return -1;
}

QStringList MaterialCategory::getCategories() {
	QStringList result;
	for( size_t i=0 ; i < static_cast<size_t>(NUM_CATEGORIES); ++i)
		result << name(static_cast<MaterialCategory::category_t>(i));
	return result;
}

}  // namespace QtExt
