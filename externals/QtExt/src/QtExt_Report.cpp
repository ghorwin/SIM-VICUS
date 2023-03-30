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
#include "QtExt_ReportSettingsBase.h"
#include "QtExt_Logger.h"

namespace QtExt {

Report::Report(ReportSettingsBase* reportSettings, int defaultFontSize) :
		m_textDocument(new QTextDocument),
		m_reportSettings(reportSettings),
		m_data(nullptr),
		m_textProperties( reportSettings->m_fontFamily, defaultFontSize, m_textDocument),
		m_scale(1.0),
		m_pageCount(0),
		m_showPageNumbers(false),
		m_headerFrame( this, m_textDocument),
		m_footerFrame( this, m_textDocument),
		m_fontFamily(reportSettings->m_fontFamily),
		m_demopix(":/Demo_quer.png")
{
	QTextOption topt = m_textDocument->defaultTextOption();
	topt.setWrapMode(QTextOption::WordWrap);
	m_textDocument->setDefaultTextOption(topt);
}

Report::~Report() {
	cleanFrameList();
	delete m_data;
}


void Report::setDocumentFont(QFont newFont){

	m_textDocument->setDefaultFont( newFont );
//#ifdef Q_OS_LINUX

//	QString css =
//			"body {"
//			"	font-family: Tahoma, Geneva, sans-serif;"
//			"	font-size: 11pt;"
//			"}"
//			"h1 { font-family:Times; font-weight:bold; font-size: large;}"
//			"h2 {"
//			"	font-size: 12pt;"
//			"	font-weight: bold;"
//			"}";
//#else
//	int fontSizePoints = newFont.pointSize();
//	QString css = QString("body { font-size:%1pt; }").arg(fontSizePoints);
//#endif
//	m_textDocument->setDefaultStyleSheet(css);

}


void Report::setHeaderFooterData(	const QString& registrationMessage,
							const QString& projectID,
							const QString& applicationName,
							const QPixmap& userLogo)
{

	m_headerFrame.setHeaderData( applicationName, registrationMessage, userLogo );
	m_footerFrame.setFooterData( projectID );

}

std::set<int> Report::frameKinds() const {
	std::set<int> res;
	for(const auto& fr : m_reportFramesRegistered) {
		res.insert(fr.first);
	}
	return res;
}

std::vector<ReportFrameBase*> Report::frameByKind(int kind) const {
	std::vector<ReportFrameBase*> res;
	for(auto it : m_reportFramesRegistered) {
		if(it.first == kind)
			res.push_back(it.second);
	}
	return res;
}


void Report::print(QPrinter * printer, QFont normalFont) {
	updateVisibility();

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

	// list must be created from this function
	m_reportFramesInfos.clear();

	// For every frame we follow these steps:
	// 1. estimate required size (height) of frame
	// 2. check if frame still fits into remaining space on current page
	//    if not, start a new page
	// 3. store frame geometry and reduce m_remainingFrame accordingly.

	// to change the order of frames appearing in the report, change the order
	// of the enumeration values in ReportFrame
	for (unsigned int i=0; i<m_reportFramesRegistered.size(); ++i) {


		m_reportFramesInfos.push_back(m_reportFramesRegistered[i]);
		QtExt::ReportFrameBase* frame = m_reportFramesRegistered[i].second;
		if (frame->m_isHidden ){
			continue;
		}


		frame->update(paintDevice, m_effectivePageSize.width());
		double h = frame->wholeFrameRect().height();

		if(frame->m_onNewPage) {
			++currentPage;
			currentFrame = mainFrame; // reset frame to full content frame
		}
		double restHeight = currentFrame.height();
		double mainHeight = mainFrame.height();

		if (restHeight < h) {
			unsigned int subFrameNumber = frame->numberOfSubFrames(paintDevice, currentFrame.height(), mainFrame.height());
			// frame is not divisible
			if(subFrameNumber == 1) {
				if(restHeight < mainHeight) {
					++currentPage;
					currentFrame = mainFrame; // reset frame to full content frame
					restHeight = mainHeight;
				}
				else {
					// frame is too high and not divisible - what to do?
				}
				setCurrentFrameInfo(m_reportFramesInfos.back(), currentFrame, currentPage);
			}
			// frame is divisible
			else {
				std::vector<ReportFrameBase*> subFrames = frame->subFrames(paintDevice, currentFrame.height(), mainFrame.height());
				Q_ASSERT(!subFrames.empty());
				for(auto subFrame : subFrames) {
					subFrame->update(paintDevice, m_effectivePageSize.width());
				}

				m_reportFramesInfos.back().m_frame = subFrames.front();
				h = m_reportFramesInfos.back().m_frame->wholeFrameRect().height();
				if(restHeight < h) {
					++currentPage;
					currentFrame = mainFrame; // reset frame to full content frame
					restHeight = mainHeight;
				}
				setCurrentFrameInfo(m_reportFramesInfos.back(), currentFrame, currentPage);
				for(size_t si=1; si<subFrames.size(); ++si) {
					++currentPage;
					currentFrame = mainFrame; // reset frame to full content frame
					m_reportFramesInfos.push_back(FrameInfo());
					m_reportFramesInfos.back().m_frame = subFrames[si];
					m_reportFramesInfos.back().m_frameType = m_reportFramesRegistered[i].first;
					setCurrentFrameInfo(m_reportFramesInfos.back(), currentFrame, currentPage);
				}
			}
		}
		else {
			setCurrentFrameInfo(m_reportFramesInfos.back(), currentFrame, currentPage);
		}

		// notify others
		emit progress(i+1);

	}
	m_pageCount = currentPage;

}

void Report::registerFrame( QtExt::ReportFrameBase* newFrame, int frameKind ){
	Q_ASSERT(newFrame != nullptr);

	// allocate memories for a new added frame
	newFrame->m_isHidden = !m_reportSettings->hasFrameId(frameKind);
	m_reportFramesRegistered.push_back( std::pair<int, QtExt::ReportFrameBase*>(frameKind, newFrame) );
//	m_reportFramesInfos.push_back( FrameInfo() );

}

std::vector<QtExt::ReportFrameBase*> Report::framesbyType(int type) {
	std::vector<QtExt::ReportFrameBase*> res;
	for( const auto& ft : m_reportFramesRegistered) {
		if(ft.first == type)
			res.push_back(ft.second);
	}
	return res;
}


void Report::cleanFrameList() {
	for(auto it = m_reportFramesRegistered.begin(); it!=m_reportFramesRegistered.end(); ++it) {
		delete it->second;
	}
	m_reportFramesRegistered.clear();
	m_reportFramesInfos.clear();
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
	for (size_t i=0, endI = m_reportFramesInfos.size(); i<endI; ++i) {

		// skip frames that are not on this page
		if (m_reportFramesInfos[i].m_pageNumber != page)
			continue;

//		Q_ASSERT_X( m_reports[i], FUNC_ID, "The frame must exist.");

		// skip hidden frames
		if (m_reportFramesInfos[i].m_frame->m_isHidden)
			continue;

#ifdef DRAW_DEBUG_FRAMES
		// debug printing of frames
		QPen framepen(Qt::blue);
		framepen.setStyle(Qt::DashDotLine);
		p->setPen(framepen);
		p->drawRect(m_frames[i]);
#endif // DRAW_DEBUG_FRAMES

		m_reportFramesInfos[i].m_frame->print(p, m_reportFramesInfos[i].m_rect);

		emit progress(i + m_reportFramesInfos.size() + 1);
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
	for(auto it=m_reportFramesRegistered.begin(); it!=m_reportFramesRegistered.end(); ++it) {
		if(it->second == frame) {
			it->second->m_isHidden = !visible;
			break;
		}
	}
}

void Report::setFrameVisibility(const std::set<int>& frameKinds, bool visible) {
	for(auto it = m_reportFramesRegistered.begin(); it!=m_reportFramesRegistered.end(); ++it) {
		auto fit = frameKinds.find(it->first);
		if(fit != frameKinds.end())
			it->second->m_isHidden = !visible;
	}
}

void Report::setFramesVisibility(int type) {
	bool visible = m_reportSettings->hasFrameId(type);
	bool mainVisible = visible;
	std::vector<QtExt::ReportFrameBase*> vect = framesbyType(type);
//	Q_ASSERT(!vect.empty());

	if(vect.size() == 1) {
		setFrameVisibility(vect.front(), visible);
	}
	else {
		for(const auto& it : vect) {
			visible = hasSpecialVisibility(it, type, mainVisible);
			setFrameVisibility(it, visible);
			visible = mainVisible;
		}
	}
}

void Report::updateVisibility() {

	setHeaderVisible(m_reportSettings->hasFrameId(ReportSettingsBase::ReportHeader));
	setFooterVisible(m_reportSettings->hasFrameId(ReportSettingsBase::ReportFooter));
	setShowPageNumbers(m_reportSettings->m_showPageNumbers);
	int typeNumber = m_reportSettings->frameTypeNumber();
	if(typeNumber > 0) {
		for(int i=2; i<typeNumber+2; ++i)
			setFramesVisibility(i);
	}
	else {
		for(auto& fr : m_reportFramesRegistered) {
			fr.second->m_isHidden = false;
		}
	}
}

void Report::setShowPageNumbers(bool enabled) {
	m_showPageNumbers = enabled;
}

void Report::setCurrentFrameInfo(FrameInfo& frameInfo, QRect& currentFrame, int currentPage) {
	int y = currentFrame.top();
	int h = frameInfo.m_frame->wholeFrameRect().height();
	frameInfo.m_rect = QRect(0, y, m_effectivePageSize.width(), h);
	currentFrame.setTop(y + h);
	frameInfo.m_pageNumber = currentPage;
}

bool Report::drawItemRect() const {
	return m_drawItemRect;
}

void Report::setDrawItemRect(bool newDrawItemRect) {
	m_drawItemRect = newDrawItemRect;
	m_headerFrame.setDrawItemRect(newDrawItemRect);
	m_footerFrame.setDrawItemRect(newDrawItemRect);
	for(auto& fi : m_reportFramesInfos) {
		fi.m_frame->setDrawItemRect(newDrawItemRect);
	}
}

void Report::setLogfile(const std::string& logfile) {
	m_logfile = logfile;
	Logger::instance().set(m_logfile);
}


} // namespace QtExt {
