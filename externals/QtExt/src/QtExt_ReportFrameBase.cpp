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

#include <fstream>

#include "QtExt_ReportFrameBase.h"

#include "QtExt_Report.h"
#include "QtExt_TextProperties.h"
#include "QtExt_Logger.h"

namespace QtExt {

ReportFrameBase::ReportFrameBase(	Report* report, QTextDocument* textDocument ) :
	m_onNewPage(false),
	m_isHidden(false),
	m_report(report),
	m_textDocument(textDocument)
{
}

ReportFrameBase::~ReportFrameBase() {
	clearSubFrames();
}

void ReportFrameBase::clearSubFrames() {
	for(ReportFrameBase* subFrame : m_currentSubFrames) {
		delete subFrame;
	}
	m_currentSubFrames.clear();
}

void ReportFrameBase::update(QPaintDevice* , double width) {
	Logger::instance() << std::string("Frame: ") + typeid(*this).name();
	qreal height = 0;
	for(auto& item : m_items) {
		if(item->isVisible()) {
			height += item->rect().height();
			Logger::instance() << item->posText();
		}
	}
	m_wholeFrameRect = QRectF(0, 0, width, height);
}

void ReportFrameBase::print(QPainter * p, const QRectF & frame) {
	QPointF pos = frame.topLeft();
	for(auto& item : m_items) {
		if(item->isVisible())
			item->draw(p, pos);
	}
}

size_t ReportFrameBase::addItem(ReportFrameItemBase* item) {
	m_items.push_back(std::shared_ptr<ReportFrameItemBase>(item));
	return m_items.size() - 1;
}

size_t ReportFrameBase::addItems(ReportFrameItemBase* itemLeft, int dist, ReportFrameItemBase* itemRight) {
	itemLeft->setNoYStep(true);
	m_items.push_back(std::shared_ptr<ReportFrameItemBase>(itemLeft));
	itemRight->setXPos(itemLeft->rect().width() + dist);
	m_items.push_back(std::shared_ptr<ReportFrameItemBase>(itemRight));
	return m_items.size() - 1;
}

ReportFrameItemBase* ReportFrameBase::item(size_t index) {
	Q_ASSERT(index < m_items.size());
	return m_items[index].get();
}

unsigned int ReportFrameBase::numberOfSubFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) const {
	return (unsigned int)lastItemOnPageList(paintDevice, heightFirst, heightRest).size();
}

std::vector<ReportFrameBase*> ReportFrameBase::subFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) {
	clearSubFrames();
	std::vector<size_t> liop = lastItemOnPageList(paintDevice, heightFirst, heightRest);
	if(liop.size() == 1)
		return m_currentSubFrames;

	size_t start = 0;
	for(size_t index : liop) {
		ReportFrameBase* frame = new ReportFrameBase(m_report, m_textDocument);
		for(size_t i=start; i<=index; ++i) {
			frame->m_items.push_back(m_items[i]);
		}
		m_currentSubFrames.push_back(frame);
		start = index + 1;
	}
	return m_currentSubFrames;
}

bool ReportFrameBase::drawItemRect() const {
	return m_drawItemRect;
}

void ReportFrameBase::setDrawItemRect(bool newDrawItemRect) {
	m_drawItemRect = newDrawItemRect;
	for(auto item : m_items) {
		item->setDrawItemRect(m_drawItemRect);
	}
}

std::vector<size_t> ReportFrameBase::lastItemOnPageList(QPaintDevice* , double heightFirst, double heightRest) const {
	// create list of heights of sections between possible page breaks
	// such a section can be called subframe
	double currentHeight = 0;
	double totalHeight = 0;
	std::vector<qreal> itemHeights(m_items.size());
	for(size_t i =0; i<m_items.size(); ++i) {
		itemHeights[i] = m_items[i]->rect().height();
	}

	std::vector<std::pair<double,size_t>> heightList;
	for(size_t i =0; i<m_items.size(); ++i) {
		qreal currItemHeight = itemHeights[i];
		currentHeight += currItemHeight;
		totalHeight += currItemHeight;
		if(m_items[i]->canPageBreakAfter()) {
			heightList.push_back({currentHeight,i});
			currentHeight = 0;
		}
	}

	// height list can be empty if no page break exist
	// in this case we have only one frame block
	if(heightList.empty()) {
		heightList.push_back({totalHeight, m_items.size() - 1});
	}
	else {
		// Check if all frames are added to height list
		// if not, add all missing frame heights
		if(heightList.back().second < m_items.size() - 1) {
			double lastHeight = 0;
			for(size_t i=heightList.back().second+1; i<m_items.size(); ++i)
				lastHeight += itemHeights[i];
			heightList.push_back({lastHeight,m_items.size()-1});
		}
	}

	// if we have only one height or the total subframe block fits in the currently available height.
	// return the number of the last subframe
	if(heightList.size() == 1 || totalHeight <= heightFirst)
		return std::vector<size_t>(1, m_items.size() - 1);

	// find out how many subframes fits on the available size of the current page
	// the first page have a different free area
	std::vector<size_t> lastItemOnPage;
	bool firstPage = true;
	currentHeight = 0;   // contains height of the current set of subframes
	for(size_t i=0; i<heightList.size(); ++i) {
		std::pair<double,size_t>& hi = heightList[i];
		currentHeight += hi.first; // add height of the current subframe to current height
		if(i==0 && hi.first >= heightFirst)
			firstPage = false;
		// check for first page - avalable height is
		if(firstPage ) {
			// height of the current set of subframes is bigger than available height
			if(currentHeight >= heightFirst) {
				// its the first subframe - add only this
				if(i==0) {
					lastItemOnPage.push_back(hi.second);
					currentHeight = 0;
				}
				// we have a set of subframes - add all except the last one
				// and set the current height to the last one
				else {
					lastItemOnPage.push_back(heightList[i-1].second);
					currentHeight = hi.first;
				}
				// the first page is filled - go further to other pages
				firstPage = false;
			}
		}
		// first page is filled check for other pages
		else {
			if(currentHeight >= heightRest) {
				if(i==0) {
					lastItemOnPage.push_back(hi.second);
					currentHeight = 0;
				}
				else {
					lastItemOnPage.push_back(heightList[i-1].second);
					currentHeight = hi.first;
				}
			}
		}
	}
	if(lastItemOnPage.empty() || lastItemOnPage.back() < heightList.back().second)
		lastItemOnPage.push_back(heightList.back().second);

	return lastItemOnPage;
}


} // namespace QtExt {
