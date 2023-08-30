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
#include <set>

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

class ReportSettingsBase;

struct ReportData {
	virtual ~ReportData() {}
};

template< class T>
struct ReportData1 : public ReportData {
	ReportData1(T* m1) :
		m_1(m1)
	{}

	T* m_1;
};

template< class T, class U>
struct ReportData2 : public ReportData {
	ReportData2(T* m1, U* m2) :
		m_1(m1),
		m_2(m2)
	{}

	T* m_1;
	U* m_2;
};

template< class T, class U, class V>
struct ReportData3 : public ReportData {
	ReportData3(T* m1, U* m2 = nullptr, V* m3 = nullptr) :
		m_1(m1),
		m_2(m2),
		m_3(m3)
	{}

	T* m_1;
	U* m_2;
	V* m_3;
};

class Report : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Report)
public:
	/*! Constructor takes a reference to the calculation data object
		to be used for printing the report.
		\param fontFamily	The font family to be used for all fonts in the report.
	*/
	explicit Report(ReportSettingsBase* reportSettings, int defaultFontSize = 11);

	/*! Destructor.
		Deletes all frames.
	*/
	~Report();

	/*! Transfer of data for showing in reports.*/
	virtual void set(ReportData* data) = 0;

	/*! Function to print the complete report onto a printer.
		All geometrical information needed is taken from the printer object.
		It is necessary to call Report::set at least once before printing in order to set all relevant informations.
	*/
	virtual void print(QPrinter * printer, QFont normalFont);

	/*! Prints the page with the given page number (1 <= page <= m_pageCount).*/
	void printPage(QPaintDevice * paintDevice, unsigned int page, QFont normalFont);

	/*! Set the default font and the font size for body in default css
		for the internal document.*/
	void setDocumentFont(QFont newFont);

	/*! Set header and footer texts. */
	void setHeaderFooterData(	const QString& registrationMessage,
								const QString& projectID,
								const QString& applicationName,
								const QPixmap& userLogo);

	/*! Return a set of all existing frame kinds.*/
	std::set<int> frameKinds() const;

	/*! Return the frames of the given kind or an empty vector in case of frame of given kind doesn't exist.*/
	std::vector<ReportFrameBase*> frameByKind(int kind) const;

	/*! Return a pointer to the internal report settings object.*/
	ReportSettingsBase* reportSettings() const {
		return m_reportSettings;
	}

	// this needs to be done to have proper init lists for constructor
protected:
	/*! Global text document. For shared use in tables and textFrames.*/
	QTextDocument*					m_textDocument;

	/*! Pointer to the report settings (can be derived class).*/
	ReportSettingsBase*				m_reportSettings;

	/*! Data for report frames to show.*/
	ReportData*						m_data;

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

	/*! Set a logfile. Logging is enabled if the logfilename is not empty.*/
	void setLogfile(const std::string& newLogfile);

	bool drawItemRect() const;
	void setDrawItemRect(bool newDrawItemRect);

signals:
	/*! Send current progress state.
		Maximum is FrameCount * 2.
	*/
	void progress(size_t) const;

public slots:


protected:
	/*! Adds a new Frame to the Report. Report takes ownership. No redraw or calculations are done. */
	void registerFrame( QtExt::ReportFrameBase* newFrame, int frameKind = 0);

	/*! Adds a new Frame to the Report. Report takes ownership. No redraw or calculations are done. */
	template<typename T>
	inline void registerFrame( QtExt::ReportFrameBase* newFrame, const T& frameKind = T()) {
		registerFrame(newFrame, static_cast<int>(frameKind));
	}

	/*! Overwrite this function in derived class.
		Function set all frames to be shown with their parameter.*/
	virtual void setFrames() = 0;

	/*! Return a vector of all frames with the given type.*/
	std::vector<QtExt::ReportFrameBase*> framesbyType(int type);

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

	/*! Set visibility of the given frame.
		\param frame Pointer to frame.
		\param visible If true footer is visible.
	*/
	void setFrameVisibility(QtExt::ReportFrameBase* frame, bool visible);

	/*! Set visibility of of all frames with the given frame kind.
		\param frameKinds set of frame kinds.
		\param visible If true footer is visible.
	*/
	void setFrameVisibility(const std::set<int>& frameKinds, bool visible);

	/*! Set visibility for frames of given frame type according report settings.
		\param type Report frame type (see ReportSettings).
	*/
	void setFramesVisibility(int type);

	/*! Return visibilty of a frame with the given type based on properties of derived ReportSettings class.
		Should be reimplemented in derived class in case such special visibilties exist.*/
	virtual bool hasSpecialVisibility(ReportFrameBase* , int , bool ) {
		return true;
	}

	/*! Updates the visibilty flags of all frames according report settings.*/
	void updateVisibility();

	/*! Set if page numbers should be shown.*/
	void setShowPageNumbers(bool enabled);

private:
	struct FrameInfo {
		FrameInfo(ReportFrameBase* frame = nullptr, int type = -1) :
			m_rect(QRect(0,0,0,0)),
			m_pageNumber(0),
			m_frame(frame),
			m_frameType(type)
		{}
		FrameInfo(std::pair<int, ReportFrameBase*> frameType) :
			m_rect(QRect(0,0,0,0)),
			m_pageNumber(0),
			m_frame(frameType.second),
			m_frameType(frameType.first)
		{}
		QRect				m_rect;
		unsigned int		m_pageNumber;
		ReportFrameBase*	m_frame;
		int					m_frameType;
	};

	void setCurrentFrameInfo(FrameInfo& frameInfo, QRect& currentFrame, int currentPage);

	/*! One common header for all sub frames */
	HeaderFrame						m_headerFrame;

	/*! One common footer for all sub frames */
	FooterFrame						m_footerFrame;

	/// The font family to be used for the report.
	QString							m_fontFamily;

	/*! Frame rects of printing elements.*/
	std::vector< FrameInfo >		m_reportFramesInfos;

	/*! Frame rect of header.*/
	QRect							m_headerRect;

	/*! Frame rect of footer.*/
	QRect							m_footerRect;

	/*! Pixmap with demo logo.*/
	QPixmap							m_demopix;

	/*! Contains the list of pointers for all report frames registered to this report. */
	std::vector< std::pair<int, ReportFrameBase*> > m_reportFramesRegistered;

	/*! Contains the list of pointers for all report frames registered to this report. */
	std::string						m_logfile;

	/*! If true a rect will be drawn around of each report item.
		It can be useful for debugging purposes.
	*/
	bool							m_drawItemRect = false;
};

} // namespace QtExt {

/*! @file QtExt_Report.h
	@brief Contains the base class for reports.
*/


#endif // QtExt_ReportH
