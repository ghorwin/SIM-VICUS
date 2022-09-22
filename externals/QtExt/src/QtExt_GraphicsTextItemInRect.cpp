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

#include "QtExt_GraphicsTextItemInRect.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace QtExt {

GraphicsTextItemInRect::GraphicsTextItemInRect(const QString &text, QGraphicsItem *parent) :
	QGraphicsTextItem(text, parent),
	m_rectFillColor(Qt::white),
	m_rectFramePen(QBrush(Qt::black), 2, Qt::SolidLine),
	m_margin(2)
{
}

void GraphicsTextItemInRect::paint(QPainter *painter,
									  const QStyleOptionGraphicsItem *option,
									  QWidget *widget)
{
	QStyleOptionGraphicsItem myOption = *option;
//	bool selected = option->state & QStyle::State_Selected;
//	myOption.state &= ~QStyle::State_Selected; // delete selected state in order to draw own one

	QRectF boundingRect = QGraphicsTextItem::boundingRect();
	painter->save();

	QRectF rect = boundingRect.adjusted(m_margin*-1, m_margin*-1,m_margin,m_margin);

	painter->setPen(m_rectFramePen);
	painter->setBrush(m_rectFillColor);
	painter->drawRect(rect);

	painter->restore();

	QGraphicsTextItem::paint(painter, &myOption, widget);

}

} // namespace QtExt
