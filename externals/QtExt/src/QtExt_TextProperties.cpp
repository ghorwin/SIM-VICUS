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

#include "QtExt_TextProperties.h"

#include <memory>

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>

namespace QtExt {

TextProperties::TextProperties(const QString& fontFamily, int fontPointSize,
							   QTextDocument* textDocument, QPaintDevice* paintDevice) :
	m_normalFont(fontFamily, fontPointSize)
{
	m_normalFont.setKerning(false);
	update(textDocument, paintDevice);
}

void TextProperties::update(QTextDocument* textDocument, QPaintDevice* paintDevice) {

	Q_ASSERT(textDocument);
	std::unique_ptr<QTextDocument> m_textDocument(textDocument->clone());
	if( paintDevice)
		m_textDocument->documentLayout()->setPaintDevice(paintDevice);
	m_textDocument->setDefaultFont(m_normalFont);

	QString testString = QString::fromWCharArray(L"QWERTZUIOPÃASDFGHJKLÃÃYXCVBNM;:_qwertzuiopÃ¼asdfghjklÃ¶Ã¤yxcvbnm,.-");

	m_textDocument->setHtml(testString);
	m_textDocument->idealWidth();		// forces creating of layout and text lines

	QTextBlock block = m_textDocument->firstBlock();
	QTextLayout* layout = block.layout();
	m_textNormal.height = layout->lineAt(0).height();
	m_textNormal.ascent = layout->lineAt(0).ascent();
	m_textNormal.descent = layout->lineAt(0).descent();

	m_textDocument->setHtml(QString("<h1>%1</h1>").arg(testString));
	m_textDocument->idealWidth();		// forces creating of layout and text lines
	block = m_textDocument->firstBlock();
	layout = block.layout();
	m_textH1.height = layout->lineAt(0).height();
	m_textH1.ascent = layout->lineAt(0).ascent();
	m_textH1.descent = layout->lineAt(0).descent();

	m_textDocument->setHtml(QString("<h2>%1</h2>").arg(testString));
	m_textDocument->idealWidth();		// forces creating of layout and text lines
	block = m_textDocument->firstBlock();
	layout = block.layout();
	m_textH2.height = layout->lineAt(0).height();
	m_textH2.ascent = layout->lineAt(0).ascent();
	m_textH2.descent = layout->lineAt(0).descent();

	m_textDocument->setHtml(QString("<h3>%1</h3>").arg(testString));
	m_textDocument->idealWidth();		// forces creating of layout and text lines
	block = m_textDocument->firstBlock();
	layout = block.layout();
	m_textH3.height = layout->lineAt(0).height();
	m_textH3.ascent = layout->lineAt(0).ascent();
	m_textH3.descent = layout->lineAt(0).descent();

	m_textDocument->setHtml(QString("<h4>%1</h4>").arg(testString));
	m_textDocument->idealWidth();		// forces creating of layout and text lines
	block = m_textDocument->firstBlock();
	layout = block.layout();
	m_textH4.height = layout->lineAt(0).height();
	m_textH4.ascent = layout->lineAt(0).ascent();
	m_textH4.descent = layout->lineAt(0).descent();

	m_textDocument->setHtml(QString("<small>%1</small>").arg(testString));
	m_textDocument->idealWidth();		// forces creating of layout and text lines
	block = m_textDocument->firstBlock();
	layout = block.layout();
	m_textSmall.height = layout->lineAt(0).height();
	m_textSmall.ascent = layout->lineAt(0).ascent();
	m_textSmall.descent = layout->lineAt(0).descent();
}

} // namespace QtExt {
