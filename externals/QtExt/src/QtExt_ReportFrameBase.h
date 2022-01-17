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

#ifndef QtExt_ReportFrameBaseH
#define QtExt_ReportFrameBaseH

#include <QRectF>

#include <memory>
#include <vector>

#include <QtExt_ReportFrameItemBase.h>

class QTextDocument;
class QPaintDevice;
class QPainter;

namespace QtExt {

class Report;

/*! Abstract base class for report frames.*/
class ReportFrameBase {
public:
	/*! Only constructor.
		\param report Reference to report (owner).
		\param fontFamily Font family name used for creating standard font.
		\param fontPointSize Fonte size for normal font in points.
		\param textDocument Current textDocument responsible for rendering text. Necessary for derived classes.
	*/
	ReportFrameBase(	Report* report,
						QTextDocument* textDocument);

	/*! Destructor.
		Implemented because this class is a abstract base class.
	*/
	virtual ~ReportFrameBase() {}

	/*! Prepares frame for drawing. Updates the m_wholeFrameRect.
		\param paintDevice Paint device in order to calculate correct sizes.
		\param width Maximum possible width for drawing.
		Base version of update create a new wholeFrameRect based on the existing item list.
	*/
	virtual void update(QPaintDevice* paintDevice, double width);

	/*! Renders the complete frame with the given painter.
		\param p Painter for drawing.
		\param frame External frame for drawing (maximum width, height and position).
	*/
	virtual void print(QPainter * p, const QRectF & frame);

	/*! Returns the whole frame rect necessary for drawing.
		The size of the frame will be calculated by calling update for the given maximum width.
	*/
	QRectF	wholeFrameRect() const { return m_wholeFrameRect; }

	/*! Add a item to the item list.*/
	size_t addItem(ReportFrameItemBase* item);

	size_t itemCount() const { return m_items.size(); }

	ReportFrameItemBase* item(size_t index);

	/*! This flag marks if a report shoud be hidden or not in the report manager class. */
	bool			m_isHidden;

protected:
	Report*												m_report;				///< Reference to report.
	QTextDocument*										m_textDocument;			///< Pointer to text document object responsible for rendering text and images.
	QRectF												m_wholeFrameRect;		///< Rectangle of the whole area.
	std::vector<std::unique_ptr<ReportFrameItemBase>>	m_items;				///< List of items to be drawn

};

} // namespace QtExt {

#endif // QtExt_ReportFrameBaseH
