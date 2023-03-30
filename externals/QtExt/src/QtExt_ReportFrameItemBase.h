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

#ifndef QtExt_ReportFrameItemBaseH
#define QtExt_ReportFrameItemBaseH

#include <QRectF>

#include <string>

class QPainter;
class QPaintDevice;

namespace QtExt {

class ReportFrameItemBase
{
public:
	ReportFrameItemBase(QPaintDevice* paintDevice, double width, double spaceAfter = 0, double spaceBefore = 0, bool canPageBreakAfter = false);

	/*! Create a surrounding rect based on the current settings and the given paintDevice and width.
		This base version create a rect with the whole width and the height based on space before and after.
		The currentRect must be calculated from each derived class before calling this function by implementing setCurrentRect().
	*/
	QRectF rect();

	/*! Set the start position - draw item by using drawItem() function - set end position.
		The drawItem function must be implemented by each derived class.
	*/
	void draw(QPainter* painter, QPointF& pos);

	/*! Set the visibility of the item. A non visible item will not be considered in frame calculation and will not be printed by the parent report frame.*/
	void setVisible(bool visible);

	/*! Return visibility of the item. This function will be used from the draw and rect calculation function in ReportFrameBase.*/
	bool isVisible() const { return m_visible; }

	/*! Set space before item.*/
	void setSpaceBefore(double spaceBefore) { m_spaceBefore = spaceBefore; }

	/*! Set space after item.*/
	void setSpaceAfter(double spaceAfter) { m_spaceAfter = spaceAfter; }

	/*! Set space after item.*/
	void setXPos(double pos) { m_xPos = pos; }

	/*! Set flag if the item should'n perform a Y step in setEndPos.
		Can be used in order to draw items side by side.
	*/
	void setNoYStep(bool noYStep);

	/*! Return break capability after the item. If true a page break is possible.*/
	bool canPageBreakAfter() const { return m_canPageBreakAfter; }

	/*! generates a desription text for positions and sizes.*/
	virtual std::string	posText() const;

	bool drawItemRect() const;
	void setDrawItemRect(bool newDrawItemRect);

protected:

	/*! Draw the item with the given painter at the given position.
		Function must be implemented in derived classes.
	*/
	virtual void drawItem(QPainter* painter, QPointF& pos) = 0;

	/*! Each derived class must calculate the rect of the complete content.
		This rect should not include space before and after.
	*/
	virtual void setCurrentRect() = 0;

	/*! Set internal variables based on the given ones.
	*/
	void setInternals(ReportFrameItemBase* item) const;

	QPaintDevice*	m_paintDevice;
	QRectF			m_currentRect;
	double			m_width;
	bool			m_visible;
	double			m_spaceBefore;
	double			m_spaceAfter;
	double			m_xPos;
	bool			m_canPageBreakAfter;
	bool			m_noYStep = false;

private:

	/*! Set start position. Should be called from draw function in each derived class before dtawing.*/
	void setStartPos(QPointF& pos);

	/*! Set end position. Should be called from draw function in each derived class after dtawing.*/
	void setEndPos(QPointF& pos);

	double			m_oldXPos;
	bool			m_drawItemRect = false;
};

} // namespace QtExt

#endif // QtExt_ReportFrameItemBaseH
