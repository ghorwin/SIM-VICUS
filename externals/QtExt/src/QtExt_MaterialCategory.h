/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

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

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
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
