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

#ifndef QtExt_CellSizeFormaterH
#define QtExt_CellSizeFormaterH

#include <QtGlobal>

namespace QtExt {

	/*! \brief Struct CellSizeFormater contains the table cell format option and size.*/
	/*! It is used for row and column size formating.*/
	struct CellSizeFormater {
		/*! Size formating type.*/
		enum FormatType {
							TableWidth,		///< ??
							AutoMinimum,	///< Automatic calculation of minimum cell rect for given text.
							Fixed,			///< Cell size is fixed.
							Unknown			///< Unknown property is used for non valid cell size.
						};

		qreal		m_size;			///< Current size.
		qreal		m_fixedSize;	///< Size for fixed mode.
		FormatType	m_format;		///< Size format type.

		/*! Default constructor.*/
		CellSizeFormater() :
			m_size(0.0),
			m_fixedSize(0.0),
			m_format(TableWidth)
		{}

		/*! Constructor with predefined format.*/
		CellSizeFormater(FormatType format) :
			m_size(0.0),
			m_fixedSize(0.0),
			m_format(format)
		{}
	};

} // namespace QtExt

#endif // QtExt_CellSizeFormaterH
