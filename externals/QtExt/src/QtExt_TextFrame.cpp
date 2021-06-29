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

#include <QTextBlock>
#include <QTextLayout>
#include <QPointF>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QDebug>

#include "QtExt_TextFrame.h"

namespace QtExt {

TextFrame::TextFrame(QTextDocument* textDocument, QObject *parent) :
		QObject(parent),
		m_textDocument(textDocument),
		m_textDocumentOwner(false),
		m_leftMargin(0),
		m_rightMargin(0),
		m_backgroundColor(Qt::transparent),
		m_fontColor(Qt::black)
{
	if( m_textDocument == 0) {
		m_textDocument = new QTextDocument;
		m_textDocumentOwner = true;
	}
}

TextFrame::~TextFrame() {
	if(m_textDocumentOwner)
		delete m_textDocument;
}

void TextFrame::setDefaultFont(const QFont& font) {
	m_textDocument->setDefaultFont(font);
}

qreal TextFrame::leftMargin() const {
	return m_leftMargin;
}

void TextFrame::setLeftMargin(qreal margin) {
	m_leftMargin = margin;
	emit changed();
}

qreal TextFrame::rightMargin() const {
	return m_rightMargin;
}

void TextFrame::setRightMargin(qreal margin) {
	m_rightMargin = margin;
	emit changed();
}

void TextFrame::setAlignment(Qt::Alignment alignment) {
	m_alignment = alignment;
}

Qt::Alignment TextFrame::alignment() const {
	return m_alignment;
}

void TextFrame::setText(const QString& text) {
	m_text = text;
	emit changed();
}

QString TextFrame::text() const {
	return m_text;
}

QColor TextFrame::backgroundColor() const {
	return m_backgroundColor;
}

void TextFrame::setBackgroundColor(const QColor& col) {
	m_backgroundColor = col;
}

QColor TextFrame::fontColor() const {
	return m_fontColor;
}

void TextFrame::setFontColor(const QColor& col) {
	m_fontColor = col;
}

void TextFrame::drawFrame(QPainter* painter, const QPointF& pos, qreal width) {
	QPaintDevice* device = painter->device();
	if( !device)
		return;

	m_textDocument->documentLayout()->setPaintDevice(painter->device());
	m_textDocument->setTextWidth(width);
	m_textDocument->setDocumentMargin(0);
	m_textDocument->setHtml(m_text);

	m_textDocument->idealWidth();		// forces creating of layout and text lines
//	QPointF internalPos = pos;
	painter->save();
	// set background color
	painter->setBackgroundMode(Qt::OpaqueMode);
	painter->setBackground(QBrush(m_backgroundColor));
	// set text/font color
	painter->setPen(m_fontColor);

	painter->translate(pos);
	m_textDocument->drawContents(painter);

//	for( QTextBlock block = m_textDocument->begin(); block != m_textDocument->end(); block = block.next()) {
//		if( !block.isValid())
//			return;
//		QTextLayout* layout = block.layout();
//		if( !layout || layout->lineCount() == 0)
//			return;
//		QTextOption option = layout->textOption();
//		option.setAlignment(m_alignment);
//		layout->setTextOption(option);
//		QRectF bbrect = m_textDocument->documentLayout()->blockBoundingRect(block);
//		qreal height = bbrect.height();

//		layout->draw(painter, internalPos);
//		internalPos.ry() += height;
//	}
	painter->restore();
}

QRectF TextFrame::frameRect(QPaintDevice* paintDevice, qreal width) {
	QRectF result;
	if( !paintDevice)
		return result;
	if( width <= 0)
		width = paintDevice->width();

	m_textDocument->documentLayout()->setPaintDevice(paintDevice);
	result = textRect(m_text, width);
	return result;
}

QRectF TextFrame::textRect(const QString& html, qreal width) const {
	if( html.isEmpty())
		return QRectF();
	m_textDocument->setTextWidth(width);
	m_textDocument->setDocumentMargin(0);
	m_textDocument->setHtml(html);
	qreal w1 = m_textDocument->idealWidth();		// forces creating of layout and text lines

	QSizeF ds = m_textDocument->documentLayout()->documentSize();
	QRectF rect(0,0, w1, ds.height());
	return rect;
}

} // namespace QtExt
