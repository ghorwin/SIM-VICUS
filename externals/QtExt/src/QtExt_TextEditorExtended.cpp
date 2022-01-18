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

#include "QtExt_TextEditorExtended.h"

#include <QPainter>
#include <QTextBlock>
#include <QFontDatabase>


namespace QtExt {

void TextEditorExtended::ColumnMarker::updatePosition() {
	m_startLinePos = m_startLine * m_edit->m_lineHeight + m_edit->m_topMargin;
	m_startColumnPos = m_startColumn * m_edit->m_charWidth + m_edit->m_leftMargin;
	m_endColumnPos = m_startColumnPos + m_width * m_edit->m_charWidth + m_edit->m_leftMargin;
}

TextEditorExtended::TextEditorExtended(QWidget *parent) :
	QPlainTextEdit(parent),
	m_lineNumberArea(new LineNumberAereaWidget(this)),
	m_headerEndRow(0)
{

	connect(this, &TextEditorExtended::blockCountChanged, this, &TextEditorExtended::updateLineNumberAreaWidth);
	connect(this, &TextEditorExtended::updateRequest, this, &TextEditorExtended::updateLineNumberArea);
	connect(this, &TextEditorExtended::cursorPositionChanged, this, &TextEditorExtended::highlightCurrentLine);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
	setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	setLineWrapMode(QPlainTextEdit::NoWrap);

	updateHeightWidth();
}

int TextEditorExtended::lineNumberAreaWidth() {
	int digits = 1;
	int max = std::max(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}

#if QT_VERSION < 0x050B00
	int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
#else // QT_VERSION < 0x051100
	int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
#endif
	return space;
}

void TextEditorExtended::colorRows(unsigned int startRow, unsigned int endRow, QColor color) {
	if(startRow >= endRow || (int)endRow >= blockCount()) {
		clearRowColoring();
		return;
	}
	m_headerEndRow = (int)endRow;

	QTextEdit::ExtraSelection selection;

	selection.format.setBackground(color);
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	selection.cursor = textCursor();
	selection.cursor.clearSelection();
	selection.cursor.setPosition(0);
	selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, startRow);
	selection.cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, endRow-startRow);

	setExtraSelections(QList<QTextEdit::ExtraSelection>() << selection);
}

void TextEditorExtended::clearRowColoring() {
	setExtraSelections(QList<QTextEdit::ExtraSelection>());
	textCursor().clearSelection();
}

void TextEditorExtended::addColumnMarker(int row, int col, int width, const QColor& color) {
	m_columnMarkers.append(ColumnMarker(this, row, col, width));
	m_columnMarkers.back().m_color = color;
	m_columnMarkers.back().updatePosition();
	repaint();
}

void TextEditorExtended::clearColumnMarker() {
	m_columnMarkers.clear();
	repaint();
}


void TextEditorExtended::updateLineNumberAreaWidth(int /* newBlockCount */) {
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditorExtended::updateLineNumberArea(const QRect &rect, int dy) {
	if (dy)
		m_lineNumberArea->scroll(0, dy);
	else
		m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void TextEditorExtended::resizeEvent(QResizeEvent *e) {
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditorExtended::paintEvent(QPaintEvent* event) {
	// Base class
	QPlainTextEdit::paintEvent(event);

	// in case of skipped header rows, draw a line
	QPainter painter(viewport());
	int topPos = 0;
	if (m_headerEndRow > 0) {
		QTextCursor cursor = textCursor();
		cursor.setPosition(0);
		cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, m_headerEndRow);
		QRect rect = cursorRect(cursor);
		int pos = rect.top();
		if(pos > 0) {
			topPos = pos;
			int totalWidth = viewport()->geometry().width();
			QPen pen(Qt::black, 3);
			painter.save();
			painter.setPen(pen);
			painter.drawLine(0, pos, totalWidth, pos);
			painter.restore();
		}
		else {
			topPos = 0;
		}
	}
	if (!m_columnMarkers.isEmpty()) {
		int totalHeight = viewport()->geometry().height();
		for(int i=0; i<m_columnMarkers.size(); ++i) {
			const ColumnMarker& cm = m_columnMarkers[i];
			QPen pen(cm.m_color, 2);
			painter.save();
			painter.setPen(pen);
			painter.drawRect(cm.m_startColumnPos, topPos,
							 cm.m_endColumnPos - cm.m_startColumnPos, totalHeight - topPos);
			painter.restore();
		}
	}
}

void TextEditorExtended::highlightCurrentLine() {
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void TextEditorExtended::lineNumberAreaPaintEvent(QPaintEvent *event) {
	QPainter painter(m_lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber);
			painter.setPen(Qt::black);
			painter.drawText(0, top, m_lineNumberArea->width(), fontMetrics().height(),
							 Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

void TextEditorExtended::updateHeightWidth() {
	QString additionalText = "AAAAAAAAAAAAAAAAAA";
	appendPlainText(additionalText);
	appendPlainText(additionalText);

	QTextCursor cursor = textCursor();
	cursor.setPosition(0);
	m_topMargin = cursorRect(cursor).top();
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
	m_lineHeight = cursorRect(cursor).top() - m_topMargin;

	cursor.setPosition(0);
	m_leftMargin = cursorRect(cursor).left();
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
	m_charWidth = cursorRect(cursor).left() - m_leftMargin;

	cursor.movePosition(QTextCursor::End);
	cursor.select(QTextCursor::LineUnderCursor);
	cursor.removeSelectedText();
	cursor.deletePreviousChar();
	cursor.movePosition(QTextCursor::End);
	cursor.select(QTextCursor::LineUnderCursor);
	cursor.removeSelectedText();
	cursor.deletePreviousChar();
}

LineNumberAereaWidget::LineNumberAereaWidget(TextEditorExtended *editor) :
	QWidget(editor),
	m_editor(editor)
{
}

QSize LineNumberAereaWidget::sizeHint() const {
		return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberAereaWidget::paintEvent(QPaintEvent *event) {
	m_editor->lineNumberAreaPaintEvent(event);
}

} // namespace QtExt
