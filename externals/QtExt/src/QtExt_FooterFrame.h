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

#ifndef QtExt_FooterFrameH
#define QtExt_FooterFrameH

#include <QCoreApplication>
#include <QDate>

#include "QtExt_Table.h"
#include "QtExt_TextFrame.h"
#include "QtExt_ReportFrameBase.h"

namespace QtExt {

class Report;

/*! \brief Class for report footer.
	Its used for configuration and printing of report footer with registration information and page numbers.
*/
class FooterFrame : public ReportFrameBase
{
	Q_DECLARE_TR_FUNCTIONS(FooterFrame)
public:
	/*! Standard constructor.*/
	FooterFrame(Report* report, QTextDocument* textDocument);

	/*! Prepares frame for drawing.
		\param paintDevice Paint device in order to calculate correct sizes.
		\param width Maximum possible width for drawing.
	*/
	virtual void		update(QPaintDevice* paintDevice, double width);

	/*! Renders the complete frame with the given painter.
		\param p Painter for drawing.
		\param frame External frame for drawing (maximum width, height and position).
		Call setPage before call this function in order to have the correct page number.
	*/
	virtual void		print(QPainter * p, const QRectF & frame);

	/*! Set the page number used for printing.
		\param page Current page number
	*/
	void setPage(int page) { m_page = page; }

	/*! Sets the common content for the page Footers.
		Texts are expected to be plain text, however, additional html tags are simply forwarded.
	*/
	void setFooterData(const QString& projectID);

private:
	int					m_page;					///< Current page.
	QFont				m_headerFooterFont;		///< Font used for footer.
	QtExt::Table		m_footerTable;			///< Table for footer text
	QString				m_projectID;			///< ProjectID
};


} // namespace QtExt {

#endif // QtExt_FooterFrameH
