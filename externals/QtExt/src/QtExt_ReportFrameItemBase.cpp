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

#include "QtExt_ReportFrameItemBase.h"

namespace QtExt {

ReportFrameItemBase::ReportFrameItemBase(QPaintDevice* paintDevice, double width, double spaceAfter, double spaceBefore) :
	m_paintDevice(paintDevice),
	m_width(width),
	m_visible(true),
	m_spaceBefore(spaceBefore),
	m_spaceAfter(spaceAfter),
	m_xPos(0),
	m_oldXPos(0)
{
}

QRectF ReportFrameItemBase::rect() {
	setCurrentRect();
	double height = m_currentRect.height() + m_spaceBefore + m_spaceAfter;
	return QRectF(m_currentRect.left(), m_currentRect.top(), m_currentRect.width(), height);
}

void ReportFrameItemBase::draw(QPainter* painter, QPointF& pos) {
	setStartPos(pos);
	drawItem(painter, pos);
	setEndPos(pos);
}

void ReportFrameItemBase::setStartPos(QPointF& pos) {
	pos.ry() += m_spaceBefore;
	m_oldXPos = pos.x();
	pos.rx() = m_xPos;
}

void ReportFrameItemBase::setEndPos(QPointF& pos) {
	pos.ry() += m_spaceAfter + m_currentRect.height();
	pos.rx() = m_oldXPos;
}

} // namespace QtExt
