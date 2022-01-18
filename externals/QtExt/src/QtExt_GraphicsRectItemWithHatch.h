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

#ifndef QtExt_GraphicsRectItemWithHatchH
#define QtExt_GraphicsRectItemWithHatchH

#include <QGraphicsRectItem>
#include <QPen>

#include <QtExt_Constants.h>

namespace QtExt {

	/*! Class similar with QGraphicsRectItem but with hatching.
		Rect uses own selection method. Currently a thick black rect is drawn.
	*/
	class GraphicsRectItemWithHatch : public QGraphicsRectItem {
	public:
		/*! Standard constructor.
			\param hatchingType Set the hatching type. HT_NoHatch draws a normal rect
			\param distance Distance between hatching elements or hatching density.
		*/
		GraphicsRectItemWithHatch(HatchingType hatchingType = HT_NoHatch, qreal distance = 1);

		/*! Paint event draws the rect and hatching if some is given.*/
		void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

		/*! Return pen for hatching lines.*/
		const QPen& hatchPen() const;

		/*! Set pen for hatching lines.*/
		void setHatchPen(const QPen& pen);

	private:
		HatchingType	m_hatchingType;	///< Type of hatching.
		qreal			m_distance;		///< Distance between lines in pixel.
		QPen			m_hatchPen;		///< Pen for drawing hatch lines.

	};

} // namespace QtExt

#endif // QtExt_GraphicsRectItemWithHatchH
