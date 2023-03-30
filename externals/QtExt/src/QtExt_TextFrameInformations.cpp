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

#include <memory>
#include <algorithm>

#include <QTextDocument>
#include <QTextLayout>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QFontMetrics>
#include <QDebug>

#include "QtExt_TextFrameInformations.h"
#include "QtExt_TextFrame.h"

namespace QtExt {

int TextFrameInformations::m_pixelStep = 5;

TextFrameInformations::TextFrameInformations() :
	m_textDocument(0)
{
}

TextFrameInformations::TextFrameInformations(QTextDocument* textDocument, const QString& text, bool adaptive) :
		m_textDocument(0)
{
	set(textDocument, text, adaptive);
}

TextFrameInformations::~TextFrameInformations() {
}

TextFrameInformations::TextFrameInformations(const TextFrameInformations& src)  :
	m_textDocument(src.m_textDocument),
	m_infoVect(src.m_infoVect),
	m_text(src.m_text)
{
}

TextFrameInformations& TextFrameInformations::operator=(TextFrameInformations src) {
	Swap(src);
	return *this;
}

void TextFrameInformations::Swap(TextFrameInformations& src) {
	std::swap(m_textDocument, src.m_textDocument);
	m_infoVect.swap(src.m_infoVect);
	std::swap(m_text, src.m_text);
}

// return current total text size
// text and width must be set before
QSizeF totalTextSize(QTextDocument* textDocument) {
	QTextBlock block = textDocument->begin();
	QRectF totalBoundingRect;
	while(block.isValid()) {
		// create rectangle that can contain rectangles of all blocks
		QRectF rect = textDocument->documentLayout()->blockBoundingRect(block);
		totalBoundingRect = totalBoundingRect.united(rect);
		block = block.next();
	}
	return totalBoundingRect.size();
}

// check if document line count equals internal block line count
// if not lyout doesn't fulfill requirements for using TextFrameInformations class
bool checkValidLayout(QTextDocument* textDocument) {
	textDocument->idealWidth();
	int blocklLineCount = 0;
	QTextBlock block = textDocument->begin();
	while(block.isValid()) {
		int blc = block.lineCount();
		// if QTextBlock doesn't support this function dont't check this
		if(blc == -1)
			return true;

		blocklLineCount += block.lineCount();
		block = block.next();
	}
	return textDocument->lineCount() == blocklLineCount;
}


void TextFrameInformations::set(QTextDocument* textDocument, const QString& text, bool adaptive) {
	m_textDocument = textDocument;
	Q_ASSERT(m_textDocument != 0);
	if( text == m_text)
		return;

	m_infoVect.clear();
	m_text = text;

	// No text - no rect needed
	if( text.isEmpty())
		return;

	m_textDocument->setHtml(m_text);
	m_textDocument->setDocumentMargin(0);

	// start with infinite width
	m_textDocument->setTextWidth(-1);
	qreal iwidth = m_textDocument->idealWidth();		// forces creating of layout and text lines
	QSizeF textSize = totalTextSize(m_textDocument);
	int linecount = m_textDocument->lineCount();
	m_infoVect.emplace_back(TextFrameInfo(linecount, textSize.height(), textSize.width(), 1e10));

	if(!adaptive)
		return;

	// now look for other widths
	qreal currentHeight = textSize.height();
	int i = textSize.width();
	while( i>10) {
		m_textDocument->setTextWidth(i - 1);
		qreal lastiwidth = iwidth;
		iwidth = m_textDocument->idealWidth();		// forces creating of layout and text lines
		textSize = totalTextSize(m_textDocument);
		if(currentHeight < textSize.height()) {
			++linecount;
			m_infoVect.emplace_back(TextFrameInfo(linecount, textSize.height(), textSize.width(), m_infoVect.back().m_minWidth - 1));
			currentHeight = textSize.height();
			if(linecount > 5)
				break;
		}
		else {
			m_infoVect.back().m_minWidth = textSize.width();
		}
		if(iwidth < lastiwidth) {
			i = iwidth;
		}
		else {
			i -= m_pixelStep;
		}
	}
}

void TextFrameInformations::setInternal(bool adaptive) {
	set(m_textDocument, m_text, adaptive);
}


TextFrameInformations::TextFrameInfo TextFrameInformations::sizeForMaximumWidth(qreal maxWidth, bool adaptive) {
	if( m_text.isEmpty())
		return TextFrameInformations::TextFrameInfo::nonValid(0);

	// create internal vector if not already done
	// first only for maximum width
	if( m_infoVect.empty())
		setInternal(adaptive);

	// if internal vector is already empty somthing serious is wrong
	if( m_infoVect.empty())
		return TextFrameInformations::TextFrameInfo::nonValid(0);

	if(maxWidth < 0) {
		Q_ASSERT(m_infoVect.front().isValid());
		return m_infoVect.front();
	}

	// look for existing size object which can be drawn into given width
	int lastValidInfoIndex = -1;
	for( size_t i=0, count=m_infoVect.size(); i<count; ++i) {
		const TextFrameInfo& info = m_infoVect[i];
		if(info.isValid()) {
			lastValidInfoIndex = i;
			if(info.m_maxWidth > maxWidth && info.m_minWidth <= maxWidth)
				return info;

		}
	}
	if(lastValidInfoIndex >= 0)
		return m_infoVect[lastValidInfoIndex];

	return TextFrameInformations::TextFrameInfo::nonValid(0);
}

} // namespace QtExt
