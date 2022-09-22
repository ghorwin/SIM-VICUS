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

#include "QtExt_GraphicsRectItemWithHatch.h"
#include "QtExt_RectHatchingFunctions.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace QtExt {

	GraphicsRectItemWithHatch::GraphicsRectItemWithHatch(HatchingType hatchingType, qreal distance) :
		m_hatchingType(hatchingType),
		m_distance(distance),
		m_hatchPen(Qt::black)
	{
		m_hatchPen.setWidth(1);
		m_hatchPen.setStyle(Qt::SolidLine);
	}

	const QPen& GraphicsRectItemWithHatch::hatchPen() const {
		return m_hatchPen;
	}

	void GraphicsRectItemWithHatch::setHatchPen(const QPen& pen) {
		m_hatchPen = pen;
		update(boundingRect());
	}

	void GraphicsRectItemWithHatch::paint(QPainter *painter,
										  const QStyleOptionGraphicsItem *option,
										  QWidget *widget)
	{
		QStyleOptionGraphicsItem myOption = *option;
		bool selected = option->state & QStyle::State_Selected;
		myOption.state &= ~QStyle::State_Selected; // delete selected state in order to draw own one

		QGraphicsRectItem::paint(painter, &myOption, widget);

		if (m_hatchingType != HT_NoHatch) {

			painter->setBrush(Qt::NoBrush);
			painter->save();

			painter->setPen(m_hatchPen);
			painter->setBrush(Qt::black);

			switch(m_hatchingType) {
				case HT_LinesHorizontal: {
					drawHorizontalLines(painter, rect(), m_distance);
					break;
				}
				case HT_LinesVertical: {
					drawVerticalLines(painter, rect(), m_distance);
					break;
				}
				case HT_CrossHatchStraight: {
					drawHorizontalLines(painter, rect(), m_distance);
					drawVerticalLines(painter, rect(), m_distance);
					break;
				}
				case HT_LinesObliqueLeftToRight: {
					drawLinesObliqueLeftToRight(painter, rect(), m_distance);
					break;
				}
				case HT_LinesObliqueRightToLeft: {
					drawLinesObliqueRightToLeft(painter, rect(), m_distance);
					break;
				}
				case HT_CrossHatchOblique: {
					drawLinesObliqueLeftToRight(painter, rect(), m_distance);
					drawLinesObliqueRightToLeft(painter, rect(), m_distance);
					break;
				}
				case HT_InsulationHatch: {
					drawInsulationHatch(painter, rect(), m_distance);
					break;
				}
				default: break;
			}

			painter->restore();
		}

		// draw own selection frame
		if( selected) {
			painter->save();
			QPen selectionPen = pen();
			qreal selwidth = selectionPen.widthF();
			selwidth *= 3;
			selectionPen.setWidthF(selwidth);
			selectionPen.setColor(Qt::black);
			painter->setPen(selectionPen);
			painter->drawRect(rect());
			painter->restore();
		}
	}
} // namespace QtExt
