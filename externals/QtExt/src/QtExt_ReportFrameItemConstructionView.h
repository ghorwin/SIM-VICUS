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

#ifndef QtExt_ReportFrameItemConstructionViewH
#define QtExt_ReportFrameItemConstructionViewH

#include "QtExt_ReportFrameItemBase.h"
#include "QtExt_ConstructionGraphicsScene.h"

namespace QtExt {

struct ConstructionViewSetup {
	QVector<ConstructionLayer>								m_layers;
	QString													m_leftLabel;
	QString													m_rightLabel;
	int														m_visibleItems = QtExt::ConstructionGraphicsScene::VI_All;
	double													m_height = -1;
	double													m_resolution = 1.0;
	QColor													m_backgroundColor = Qt::white;
	QVector<QtExt::ConstructionGraphicsScene::LineMarker>	m_lineMarker;
	QtExt::ConstructionGraphicsScene::AreaMarker			m_areaMarker;
};

class ReportFrameItemConstructionView : public QtExt::ReportFrameItemBase
{
public:
	ReportFrameItemConstructionView(const ConstructionViewSetup& constructionView, QPaintDevice* paintDevice, double width, double spaceAfter = 0, double spaceBefore = 0, bool canPageBreakAfter = false);

	~ReportFrameItemConstructionView();

	/*! Create a surrounding rect based on the current settings and the given paintDevice and width.
		This base version create a rect with the whole width and the height based on space before and after.
	*/
	virtual void setCurrentRect() override;

	/*! Draw the item with the given painter at the given position and set the position for the next item.*/
	virtual void drawItem(QPainter* painter, QPointF& pos) override;

private:
	ConstructionGraphicsScene*	m_constructionScene;
//	QRect						m_rect;
};

} // namespace QtExt

#endif // QtExt_ReportFrameItemConstructionViewH
