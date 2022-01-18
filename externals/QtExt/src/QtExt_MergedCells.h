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

#ifndef QtExt_MergedCellsH
#define QtExt_MergedCellsH

#include <QRectF>

namespace QtExt {

	/*! \brief Helper struct for cell merging.*/
	struct MergedCells {
		/*! Default constructor.*/
		MergedCells() :
			firstCol(0),
			firstRow(0),
			lastCol(0),
			lastRow(0),
			m_verticalText(false)
		{}

		/*! Constructor with full argument list.*/
		MergedCells(unsigned int fc, unsigned int fr, unsigned int lc, unsigned int lr) :
				firstCol(fc),
				firstRow(fr),
				lastCol(lc),
				lastRow(lr),
				m_verticalText(false)
		{}

		unsigned int	firstCol;		///< First column.
		unsigned int	firstRow;		///< First row.
		unsigned int	lastCol;		///< last column.
		unsigned int	lastRow;		///< Last row.
		QRectF			m_cellRect;		///< Rectangle of the whole cell area.
		QRectF			m_textRect;		///< Rectangle of the text area.
		QRectF			m_maxTextRect;	///< Maximum possible rectangle for text.
		bool			m_verticalText;	///< Hold if test is vertical or horizontal
	};

} // namespace QtExt

#endif // QtExt_MergedCellsH
