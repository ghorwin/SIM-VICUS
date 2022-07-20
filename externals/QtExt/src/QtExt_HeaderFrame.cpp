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

#include "QtExt_HeaderFrame.h"

#include <QResource>
#include <QPainter>

#include "QtExt_Report.h"
#include <QtExt_ReportSettingsBase.h>

namespace QtExt {

HeaderFrame::HeaderFrame(Report *report, QTextDocument* textDocument):
	ReportFrameBase(report, textDocument),
	m_logoMargin(5),
	m_table(textDocument, false)
{

}


void HeaderFrame::setHeaderData(const QString& applicationName,
								const QString& registrationMessage,
								const QPixmap& userLogo )
{
	m_appName = applicationName;
	m_registrationMessage = registrationMessage;
	m_userLogo = userLogo;
}


void HeaderFrame::update(QPaintDevice* paintDevice, double width) {

	// Headerframe holds a table with two columns, left holds registration message, right holds user logo (if provided)

	m_table.clear();
	m_table.setTableSize(QSize(width, m_report->m_effectivePageSize.height()));
	m_table.setColumnsRows(2,1);
	m_table.setOuterFrameWidth(0);
	m_table.setInnerFrameWidth(0);
	m_table.setSpacing(0);
	m_table.setBackgroundColor(Qt::white,true);
	m_table.setCellText(0,0,tr("%1").arg(m_appName));

	// if user logo is given, resize row of the table to hold the logo
	if( !m_userLogo.isNull() )
	{
		// determine height of logo and adjust table row height
		qreal res = (qreal)paintDevice->logicalDpiX() / m_userLogo.logicalDpiX();
		m_table.setRowSizeFormat(0, QtExt::CellSizeFormater::Fixed, m_userLogo.height() * res);
	}

	// left column holds reg message
	m_table.setCellText(0,0,m_registrationMessage);

	// draw line at bottom of table
	m_table.cell(0,0).setBorderWidth(QtExt::TableCell::BottomBorder, m_report->reportSettings()->m_innerTableFrameWidth);
	m_table.cell(1,0).setBorderWidth(QtExt::TableCell::BottomBorder, m_report->reportSettings()->m_innerTableFrameWidth);

	m_table.cell(0,0).setBottomMargin( 1 ); // 1 mm space between text and top line
	m_table.cell(1,0).setBottomMargin( 1 ); // 1 mm space between text and top line

	m_wholeFrameRect = m_table.tableRect(paintDevice, m_report->m_effectivePageSize.width()).toRect();
	m_wholeFrameRect.moveTop(0);
	m_wholeFrameRect.moveLeft(0);
	// add an empty line as space between header-frame line and following text
	m_wholeFrameRect.adjust(0, 0, 0, m_report->m_textProperties.normal().height);
}


void HeaderFrame::print(QPainter * p, const QRectF & frame) {

	m_table.drawTable(p, frame.topLeft());

	if( m_userLogo.isNull() ) // don't do anything if picture is not valid
		return;

	QPointF logoPos = frame.topRight();
	qreal res = p->device()->logicalDpiX() / (qreal)m_userLogo.logicalDpiX();
	double scaledWidth = m_userLogo.width() * res;
	double scaledHeight = m_userLogo.height() * res;

	logoPos.setX( logoPos.x() - scaledWidth);
	logoPos.setY( logoPos.y());
	p->drawPixmap(QRect(logoPos.x(), logoPos.y(), scaledWidth, scaledHeight), m_userLogo);
}

} // namespace QtExt {
