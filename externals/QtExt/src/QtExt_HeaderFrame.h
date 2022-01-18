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

#ifndef QtExt_HeaderFrameH
#define QtExt_HeaderFrameH

#include <QCoreApplication>
#include <QPixmap>
#include <QString>

#include "QtExt_Table.h"
#include "QtExt_ReportFrameBase.h"

namespace QtExt {

class Report;

/*! \brief Class for report header.
	Its used for configuration and printing of report header with registration information,
	date and possible user logo.
*/
class HeaderFrame : public ReportFrameBase {
	Q_DECLARE_TR_FUNCTIONS(DisclaimerFrame)
public:
	/*! Standard constructor.*/
	HeaderFrame(	Report* report,
					QTextDocument* textDocument);

	/*! Sets the data in the header. */
	void setHeaderData( const QString& applicationName,
						const QString& registrationMessage,
						const QPixmap& userLogo);

	/*! Updates the header text.
		Header is renered as table.
		\param paintDevice Paint device in order to calculate correct sizes.
		\param width Maximum possible width for drawing.
	*/
	virtual void		update(QPaintDevice* paintDevice, double width);

	/*! Prints the header text frame.*/
	virtual void		print(QPainter * p, const QRectF & frame);

private:
	const int			m_logoMargin;			///< Margin beside user logo
	QtExt::Table		m_table;				///< Table for header text
	QString				m_registrationMessage;	///< Message composed from registration.
	QString				m_appName;				///< Application name to be displayed in header.
	QPixmap				m_userLogo;				///< User logo to be displayed in header.
};

} // namespace QtExt {

#endif // QtExt_HeaderFrameH
