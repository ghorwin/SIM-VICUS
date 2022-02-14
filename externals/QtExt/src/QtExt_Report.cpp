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

#include "QtExt_Report.h"

#include <algorithm>

#include <QPrinter>
#include <QPainter>
#include <QApplication>
//#include <QDebug>
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QDate>
#include <QPixmap>
#include <QPaintDevice>
#include <QTextStream>
#include <QMessageBox>
#include <QTextOption>
#include <QDebug>

#include "QtExt_Table.h"
#include "QtExt_TextFrame.h"
#include "QtExt_PainterSaver.h"

namespace QtExt {

Report::Report(const QString & fontFamily, int defaultFontSize) :
		m_textDocument(new QTextDocument),
		m_textProperties( fontFamily, defaultFontSize, m_textDocument),
		m_scale(1.0),
		m_pageCount(0),
		m_showPageNumbers(false),
		m_outerTableFrameWidth(1.5),
		m_innerTableFrameWidth(0.7),
		m_headerFrame( this, m_textDocument),
		m_footerFrame( this, m_textDocument),
		m_fontFamily(fontFamily),
		m_demopix(":/Demo_quer.png")
{
}

Report::~Report() {
	cleanFrameList();
}


void Report::setDocumentFont(QFont newFont){

//	qDebug() << m_textDocument->defaultFont().family();
//	qDebug() << m_textDocument->defaultFont().pointSizeF();

	m_textDocument->setDefaultFont( newFont );
#ifdef Q_OS_LINUX

	QString css =
			"body {"
			"	font-family: Tahoma, Geneva, sans-serif;"
			"	font-size: 11pt;"
			"}"
			"h1 { font-family:Times; font-weight:bold; font-size: large;}"
			"h2 {"
			"	font-size: 12pt;"
			"	font-weight: bold;"
			"}";
#else
	int fontSizePoints = newFont.pointSize();
	QString css = QString("body { font-size:%1pt; }").arg(fontSizePoints);
#endif
//	m_textDocument->setDefaultStyleSheet(css);

//	qDebug() << m_textDocument->defaultFont().family();
//	qDebug() << m_textDocument->defaultFont().pointSizeF();
}


void Report::setHeaderFooterData(	const QString& registrationMessage,
							const QString& projectID,
							const QString& applicationName,
							const QPixmap& userLogo)
{

	m_headerFrame.setHeaderData( applicationName, registrationMessage, userLogo );
	m_footerFrame.setFooterData( projectID );

}


void Report::print(QPrinter * printer, QFont normalFont) {

	// remember pointers to function parameters
	m_textDocument->documentLayout()->setPaintDevice(printer);
	setDocumentFont(normalFont);

	// Prepare the printing by calculating the necessary frame geometries
	m_effectivePageSize = QSize(printer->pageRect().width(), printer->pageRect().height());

	// Calculate global scaling factor/resolution in pix/mm
	m_scale = m_effectivePageSize.width()/printer->paperSize(QPrinter::Millimeter).width();  // in pixel/mm

	// now calculate the frames and their location on the various pages
	updateFrames(printer);

	QPainter p;
	bool res = p.begin(printer);
	if(!res) {
		QMessageBox::critical(nullptr, tr("Cannot print report."),tr("Creating paint device failed."));
		return;
	}

	p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);

	// Do the actual printing

	// get print range from printer
	int startPage = printer->fromPage();
	int lastPage = printer->toPage();
	if (startPage == 0)
		startPage = 1;
	if (lastPage == 0)
		lastPage = m_pageCount;

#ifdef DRAW_DEBUG_FRAMES

	QPen framepen(Qt::red);
	framepen.setStyle(Qt::DashDotLine);
	p.setPen(framepen);
	p.drawRect(m_mainFrame);

	framepen.setColor(Qt::green);
	framepen.setStyle(Qt::DotLine);
	p.setPen(framepen);
	QRect pageRect(0,0, m_effectivePageSize.width(), m_effectivePageSize.height());
	p.drawRect(pageRect);

	p.setPen(Qt::black);
#endif //DRAW_DEBUG_FRAMES

	// print only selected pages
	for (int i=startPage; i<= lastPage; ++i) {
		paintPage(&p, i);
		if (i < lastPage)
			printer->newPage();
	}

	p.end();
}

void Report::printPage(QPaintDevice * paintDevice, unsigned int page, QFont normalFont) {
	// remember pointers to function parameters
	m_textDocument->documentLayout()->setPaintDevice(paintDevice);
	setDocumentFont(normalFont);

	// Prepare the printing by calculating the necessary frame geometries
	m_effectivePageSize = QSize(paintDevice->width(), paintDevice->height());

	// Calculate global scaling factor/resolution in pix/mm
	m_scale = m_effectivePageSize.width() / double(paintDevice->widthMM());  // in pixel/mm

	// now calculate the frames and their location on the various pages
	updateFrames(paintDevice);

	QPainter p;
	p.begin(paintDevice);
	paintPage(&p, page);
	p.end();
}

void Report::updateFrames(QPaintDevice* paintDevice) {

	m_textProperties.update(m_textDocument, paintDevice);

	// calculate header and footer
	if( !m_headerFrame.m_isHidden) {
		m_headerFrame.update(paintDevice, m_effectivePageSize.width());
		m_headerRect = m_headerFrame.wholeFrameRect().toRect();
	}
	else {
		m_headerRect = QRect(0,0,0,0);
	}

	if( !m_footerFrame.m_isHidden) {
		m_footerFrame.update(paintDevice, m_effectivePageSize.width());
		m_footerRect = m_footerFrame.wholeFrameRect().toRect();
	}
	else {
		m_footerRect = QRect(0,m_effectivePageSize.height(),0,0);
	}



	// calculate page frame excluding header and footer
	QRect mainFrame = QRect(0, m_headerRect.height(),
						 m_effectivePageSize.width(),
						 m_effectivePageSize.height() - m_footerRect.height() - m_headerRect.height());

	// calculate remaining frames
	QRect currentFrame = mainFrame;
	unsigned int currentPage = 1; // we start on page 1

	// For every frame we follow these steps:
	// 1. estimate required size (height) of frame
	// 2. check if frame still fits into remaining space on current page
	//    if not, start a new page
	// 3. store frame geometry and reduce m_remainingFrame accordingly.

	// to change the order of frames appearing in the report, change the order
	// of the enumeration values in ReportFrame
	for (unsigned int i=0; i<m_reports.size(); ++i) {

		if (m_reports[i]->m_isHidden ){
			m_frames[i] = QRect(0,0,0,0);
			continue;
		}

		m_reports[i]->update(paintDevice, m_effectivePageSize.width());
		double h = m_reports[i]->wholeFrameRect().height();

		if (currentFrame.height() < h || m_reports[i]->m_onNewPage) {
			++currentPage;
			currentFrame = mainFrame; // reset frame to full content frame
		}
		int y = currentFrame.top();
		m_frames[i] = QRect(0, y, m_effectivePageSize.width(), h);
		currentFrame.setTop(y + h);
		m_pageNumbers[i] = currentPage;

		// notify others
		emit progress(i+1);
	}
	m_pageCount = currentPage;
}

void Report::registerFrame( QtExt::ReportFrameBase* newFrame ){

	// allocate memories for a new added frame
	m_reports.push_back( newFrame );
	m_pageNumbers.push_back( 0 );
	m_frames.push_back( QRect() );

}

void Report::cleanFrameList() {
	for(std::vector< ReportFrameBase* >::iterator it = m_reports.begin(); it!=m_reports.end(); ++it) {
		delete *it;
	}
	m_reports.clear();
	m_pageNumbers.clear();
	m_frames.clear();
}


void Report::paintPage(QPainter * p, unsigned int page) {

//	const char * const FUNC_ID = "[Report::paintPage]";

	if ( !m_headerFrame.m_isHidden ) {
		m_headerFrame.print(p, m_headerRect);
	}

	if (!m_footerFrame.m_isHidden ) {
		m_footerFrame.setPage(page);
		m_footerFrame.print(p, m_footerRect);
	}

	// loop over all frames
	for (size_t i=0, endI = m_reports.size(); i<endI; ++i) {

		// skip frames that are not on this page
		if (m_pageNumbers[i] != page)
			continue;

//		Q_ASSERT_X( m_reports[i], FUNC_ID, "The frame must exist.");

		// skip hidden frames
		if (m_reports[i]->m_isHidden)
			continue;

#ifdef DRAW_DEBUG_FRAMES
		// debug printing of frames
		QPen framepen(Qt::blue);
		framepen.setStyle(Qt::DashDotLine);
		p->setPen(framepen);
		p->drawRect(m_frames[i]);
#endif // DRAW_DEBUG_FRAMES

		m_reports[i]->print(p, m_frames[i]);

		emit progress(i + m_reports.size() + 1);
	}

#ifdef TYPE_DEMO
	QRect demoframe = QRect(0, m_headerRect.height(),
						 m_effectivePageSize.width(),
						 m_effectivePageSize.height() - m_footerRect.height() - m_headerRect.height());
	qreal opacity = p->opacity();
	p->setOpacity(0.5);
	p->drawPixmap(demoframe, m_demopix, m_demopix.rect());
	p->setOpacity(opacity);
#endif
}

void Report::setHeaderVisible(bool visible) {
	m_headerFrame.m_isHidden = !visible;
}

void Report::setFooterVisible(bool visible) {
	m_footerFrame.m_isHidden = !visible;
}

void Report::setFrameVisibility(QtExt::ReportFrameBase* frame, bool visible) {
	std::vector< ReportFrameBase* >::iterator fit = std::find(m_reports.begin(), m_reports.end(), frame);
	if(fit != m_reports.end())
		(*fit)->m_isHidden = !visible;
}

void Report::setShowPageNumbers(bool enabled) {
	m_showPageNumbers = enabled;
}

} // namespace QtExt {
