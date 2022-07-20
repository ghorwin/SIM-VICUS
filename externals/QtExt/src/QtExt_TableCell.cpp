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

#include "QtExt_TableCell.h"

#include "QtExt_Table.h"

namespace QtExt {

TableCell::TableCell(qreal width) :
		m_leftBorderWidth(width),
		m_rightBorderWidth(width),
		m_topBorderWidth(width),
		m_bottomBorderWidth(width),
		m_leftMargin(0),
		m_rightMargin(0),
		m_topMargin(0),
		m_bottomMargin(0),
		m_backgroundColor(Qt::transparent),
		m_merged(false),
		m_verticalOffset(0.0),
		m_verticalText(false)
{
}

TableCell::TableCell(const TableCell& src)  :
	m_leftBorderWidth(src.m_leftBorderWidth),
	m_rightBorderWidth(src.m_rightBorderWidth),
	m_topBorderWidth(src.m_topBorderWidth),
	m_bottomBorderWidth(src.m_bottomBorderWidth),
	m_leftMargin(src.m_leftMargin),
	m_rightMargin(src.m_rightMargin),
	m_topMargin(src.m_topMargin),
	m_bottomMargin(src.m_bottomMargin),
	m_alignment(src.m_alignment),
	m_text(src.m_text),
	m_textRect(src.m_textRect),
	m_maxTextRect(src.m_maxTextRect),
	m_cellRect(src.m_cellRect),
	m_backgroundColor(src.m_backgroundColor),
	m_merged(src.m_merged),
	m_verticalOffset(src.m_verticalOffset),
	m_textInfos(src.m_textInfos),
	m_verticalText(src.m_verticalText)
{
}

TableCell& TableCell::operator=(TableCell src) {
	Swap(src);
	return *this;
}

void TableCell::Swap(TableCell& src) {
	std::swap(m_leftBorderWidth, src.m_leftBorderWidth);
	std::swap(m_rightBorderWidth, src.m_rightBorderWidth);
	std::swap(m_topBorderWidth, src.m_topBorderWidth);
	std::swap(m_bottomBorderWidth, src.m_bottomBorderWidth);
	std::swap(m_leftMargin, src.m_leftMargin);
	std::swap(m_rightMargin, src.m_rightMargin);
	std::swap(m_topMargin, src.m_topMargin);
	std::swap(m_bottomMargin, src.m_bottomMargin);
	std::swap(m_alignment, src.m_alignment);
	std::swap(m_text, src.m_text);
	std::swap(m_textRect, src.m_textRect);
	std::swap(m_maxTextRect, src.m_maxTextRect);
	std::swap(m_cellRect, src.m_cellRect);
	std::swap(m_backgroundColor, src.m_backgroundColor);
	std::swap(m_merged, src.m_merged);
	std::swap(m_verticalOffset, src.m_verticalOffset);
	std::swap(m_textInfos, src.m_textInfos);
	std::swap(m_verticalText, src.m_verticalText);
}

void TableCell::setBorderWidth(TableCell::CellBorder border, qreal width) {
	Q_ASSERT(width >= 0);

	if( border & LeftBorder)
		m_leftBorderWidth = width;
	if( border & RightBorder)
		m_rightBorderWidth = width;
	if( border & TopBorder)
		m_topBorderWidth = width;
	if( border & BottomBorder)
		m_bottomBorderWidth = width;
}

qreal TableCell::borderWidth(TableCell::CellBorder border) const {
	switch(border) {
		case LeftBorder: return m_leftBorderWidth;
		case RightBorder: return m_rightBorderWidth;
		case TopBorder: return m_topBorderWidth;
		case BottomBorder: return m_bottomBorderWidth;
		default: return 0;
	}
}

void TableCell::setAlignment(Qt::Alignment alignment) {
	m_alignment = alignment;
}

Qt::Alignment TableCell::alignment() const {
	return m_alignment;
}

void TableCell::setText(const QString& text) {
	m_text = text;
}

QString TableCell::text() const {
	return m_text;
}

QRectF TableCell::textRect() const {
	return m_textRect;
}

void TableCell::setTextRect(const QRectF& rect) {
	m_textRect = rect;
}

QRectF TableCell::maxTextRect() const {
	return m_maxTextRect;
}

void TableCell::setMaxTextRect(const QRectF& rect) {
	m_maxTextRect = rect;
}

QSizeF TableCell::textSize(qreal maxWidth, bool adaptive) {
	QSizeF size = m_textInfos.sizeForMaximumWidth(maxWidth, adaptive).size();
	if(m_verticalText)
		size.transpose();
	return size;
}

qreal TableCell::textMaxWidth(qreal maxWidth, bool adaptive) {
	return m_textInfos.sizeForMaximumWidth(maxWidth, adaptive).m_maxWidth;
}

QRectF TableCell::cellRect() const {
	return m_cellRect;
}

void TableCell::setCellRect(const QRectF& rect) {
	m_cellRect = rect;
}

QColor TableCell::backgroundColor() const {
	return m_backgroundColor;
}

void TableCell::setBackgroundColor(const QColor& col) {
	m_backgroundColor = col;
}

qreal TableCell::leftMargin() const {
	return m_leftMargin;
}

void TableCell::setLeftMargin(qreal margin) {
	m_leftMargin = margin;
}

qreal TableCell::rightMargin() const {
	return m_rightMargin;
}

void TableCell::setRightMargin(qreal margin) {
	m_rightMargin = margin;
}

qreal TableCell::topMargin() const {
	return m_topMargin;
}

void TableCell::setTopMargin(qreal margin) {
	m_topMargin = margin;
}

qreal TableCell::bottomMargin() const {
	return m_bottomMargin;
}

void TableCell::setBottomMargin(qreal margin) {
	m_bottomMargin = margin;
}

qreal TableCell::verticalOffset() const {
	return m_verticalOffset;
}

void TableCell::setVerticalOffset(qreal offset) {
	m_verticalOffset = offset;
}

void TableCell::setMerged(bool merged) {
	m_merged = merged;
}

void TableCell::setVerticalText(bool vertical) {
	m_verticalText = vertical;
}

void TableCell::setPaintProperties(QTextDocument* textDocument, bool adaptive) {
	m_textInfos.set(textDocument, m_text,adaptive);
}


} // namespace QtExt
