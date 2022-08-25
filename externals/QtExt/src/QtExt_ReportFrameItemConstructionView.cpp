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

#include "QtExt_ReportFrameItemConstructionView.h"

#include <QPrinter>

namespace QtExt {

ReportFrameItemConstructionView::ReportFrameItemConstructionView(const ConstructionViewSetup& constructionViewSetup, QPaintDevice* paintDevice,
																 double width, double spaceAfter, double spaceBefore, bool canPageBreakAfter) :
	ReportFrameItemBase(paintDevice, width, spaceAfter, spaceBefore, canPageBreakAfter),
	m_constructionScene(new ConstructionGraphicsScene(false, paintDevice))
{
	double height = constructionViewSetup.m_height <= 0 ? width / 2.0 : constructionViewSetup.m_height;
//	width *= constructionViewSetup.m_resolution;
//	height *= constructionViewSetup.m_resolution;
	m_currentRect = QRect(0,0,width, height);
	for(const auto& line : constructionViewSetup.m_lineMarker) {
		m_constructionScene->addLinemarker(line);
	}
	if(constructionViewSetup.m_areaMarker.valid())
	m_constructionScene->setAreaMarker(constructionViewSetup.m_areaMarker);
	m_constructionScene->setBackground(constructionViewSetup.m_backgroundColor);
	m_constructionScene->setup(m_currentRect.toRect(), paintDevice, constructionViewSetup.m_resolution,
							  constructionViewSetup.m_layers, constructionViewSetup.m_leftLabel, constructionViewSetup.m_rightLabel,
							  constructionViewSetup.m_visibleItems);
}

ReportFrameItemConstructionView::~ReportFrameItemConstructionView() {
	delete m_constructionScene;
}


void ReportFrameItemConstructionView::setCurrentRect() {
}

void ReportFrameItemConstructionView::drawItem(QPainter* painter, QPointF& pos) {
	QRect drect(pos.x(), pos.y(), m_currentRect.width(), m_currentRect.height());
	m_constructionScene->render(painter, drect);
}

} // namespace QtExt
