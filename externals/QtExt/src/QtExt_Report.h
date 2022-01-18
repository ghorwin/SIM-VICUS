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

#ifndef QtExt_ReportH
#define QtExt_ReportH

#include <QObject>
#include <QSize>
#include <QRect>
#include <QFont>
//#include <QLabel>

#include <vector>

#include "QtExt_ReportFrameBase.h"
#include "QtExt_HeaderFrame.h"
#include "QtExt_FooterFrame.h"
#include "QtExt_TextProperties.h"

class QWidget;
class QPrinter;
class QPainter;
class QTextDocument;
class QPaintDevice;

namespace QtExt {

class Report : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Report)
public:
	/*! Constructor takes a reference to the calculation data object
		to be used for printing the report.
		\param fontFamily	The font family to be used for all fonts in the report.
	*/
	explicit Report(const QString & fontFamily, int defaultFontSize = 11);

	/*! Destructor.
		Deletes all frames.
	*/
	~Report();

	/*! Function to print the complete report onto a printer.
		All geometrical information needed is taken from the printer object.
		It is necessary to call Report::set at least once before printing in order to set all relevant informations.
	*/
	virtual void print(QPrinter * printer, QFont normalFont);

	/*! Prints the page with the given page number (1 <= page <= m_pageCount).*/
	void printPage(QPaintDevice * paintDevice, unsigned int page, QFont normalFont);

//	/*! Returns the current page size.*/
//	QSize effectivePageSize() const;

	/*! Set the default font and the font size for body in default css
		for the internal document.*/
	void setDocumentFont(QFont newFont);

	/*! Set header and footer texts. */
	void setHeaderFooterData(	const QString& registrationMessage,
								const QString& projectID,
								const QString& applicationName,
								const QPixmap& userLogo);

	// this needs to be done to have proper init lists for constructor
protected:
	/*! Global text document. For shared use in tables and textFrames.*/
	QTextDocument*					m_textDocument;

public:
	/*! Globally defined text properties for the whole report. */
	TextProperties					m_textProperties;

	/*! Global scale factor in pixel/mm (mainly used for diagrams).*/
	double							m_scale;

	/*! The size available for printing on each page.*/
	QSize							m_effectivePageSize;

	/*! Total number of pages as calculated by calculateFrames().*/
	unsigned int					m_pageCount;

	bool							m_showPageNumbers;		///< If true page numbers will be shown in footer.

	qreal							m_outerTableFrameWidth;	///< Pen width for outer table frame line.

	qreal							m_innerTableFrameWidth;	///< Pen width for inner table frame line.

signals:
	/*! Send current progress state.
		Maximum is FrameCount * 2.
	*/
	void progress(size_t) const;

public slots:


protected:
	/*! Adds a new Frame to the Report. Report takes ownership. No redraw or calculations are done. */
	void registerFrame( QtExt::ReportFrameBase* newFrame );

	/*! Clean the current frame list and delete all frames.*/
	void cleanFrameList();

	/*! Prints the page with the given page number (1 <= page <= m_pageCount).*/
	void paintPage(QPainter * p, unsigned int page);

	/*! Calculates the sizes and locations of the individual frames
		that make up the report.
		Setting visibilty of individual frames should be done before.
		e.g. in a overloaded update function in  a derived class.
	*/
	void updateFrames(QPaintDevice* paintDevice);

	/*! Set visibility of header.
		\param visible If true header is visible.
	*/
	void setHeaderVisible(bool visible);

	/*! Set visibility of footer.
		\param visible If true footer is visible.
	*/
	void setFooterVisible(bool visible);

	/*! Set visibility of footer.
		\param frame Pointer to frame.
		\param visible If true footer is visible.
	*/
	void setFrameVisibility(QtExt::ReportFrameBase* frame, bool visible);

	/*! Set if page numbers should be shown.*/
	void setShowPageNumbers(bool enabled);


private:
	/*! One common header for all sub frames */
	HeaderFrame						m_headerFrame;

	/*! One common footer for all sub frames */
	FooterFrame						m_footerFrame;

	/// The font family to be used for the report.
	QString							m_fontFamily;

	/*! Frame rects of printing elements.*/
	std::vector< QRect >			m_frames;

	/*! Frame rect of header.*/
	QRect							m_headerRect;

	/*! Frame rect of footer.*/
	QRect							m_footerRect;

	/*! Page numbers of the various frames.*/
	std::vector< unsigned int >		m_pageNumbers;

	/*! Pixmap with demo logo.*/
	QPixmap							m_demopix;

	/*! contains the list of pointers for all reports. */
	std::vector< ReportFrameBase* > m_reports;

};

} // namespace QtExt {

/*! @file QtExt_Report.h
	@brief Contains the base class for reports.
*/


#endif // QtExt_ReportH
