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

#ifndef QtExt_MaterialCategoryH
#define QtExt_MaterialCategoryH

#include <QString>
#include <QCoreApplication>

namespace QtExt{

/*! Wraps all categories and handles category name/index conversions.*/
class MaterialCategory {
	Q_DECLARE_TR_FUNCTIONS(MaterialCategory)
public:
	/*! Defines category types.*/
	enum category_t {
		COATING,
		PLASTER,
		BRICK,
		NATURAL_STONES,
		CONCRETE,
		INSULATION,
		BUILDING_BOARD,
		TIMBER,
		NATURAL_MATERIALS,
		SOIL,
		CLADDING,
		FOIL,
		MISCELLANEOUS,
		NUM_CATEGORIES
	};

	/*! Default constructor, creates a "Miscellaneous" category.*/
	MaterialCategory();

	/*! Constructor, creates a MaterialCategory of type 'cat'.*/
	MaterialCategory(category_t cat);

	/*! The index of the material category.*/
	category_t idx;

	/*! Returns a descriptive, internationalized name for a given
		category index.*/
	QString toString() const { return name(idx); }

	/*! Returns a descriptive, internationalized name for a given
		category index.*/
	static QString name(category_t idx);

	/*! Returns the category index for a given string.*/
	static int getIndex(const QString&);

	/*! returns a list of all category names.*/
	static QStringList getCategories();

	/*! equal operator.*/
	friend bool operator==(const MaterialCategory& lhs, const MaterialCategory& rhs) 	{
		return lhs.idx == rhs.idx;
	}

	/*! not equal operator.*/
	friend bool operator!=(const MaterialCategory& lhs, const MaterialCategory& rhs) 	{
		return lhs.idx != rhs.idx;
	}
};

}  // namespace QtExt

/*! @file QtExt_MaterialCategory.h
	@brief Contains the class MaterialCategory.
*/

#endif // QtExt_MaterialCategoryH
