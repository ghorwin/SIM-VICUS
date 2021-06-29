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

		QGraphicsRectItem::paint(painter, &myOption, widget);

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
