#ifndef QtExt_ReportFrameLongTableH
#define QtExt_ReportFrameLongTableH


#include <QApplication>

#include "QtExt_Table.h"
#include "QtExt_ReportFrameBase.h"
#include "QtExt_TextFrame.h"


namespace QtExt {

/*! Frame contains a table with one header. If the table is too long for the existing space it can be splitted into sub-tables.
	One must derive a new class from this one in order to use it. The derived class must implement the functions for configuration header and table.
*/
class ReportFrameLongTable : public QtExt::ReportFrameBase {
	Q_DECLARE_TR_FUNCTIONS(ReportFrameLongTable)
	Q_DISABLE_COPY(ReportFrameLongTable)
public:
	/*! Only constructor.
		\param report Reference to report (owner).
		\param textDocument Current textDocument responsible for rendering text. Necessary for derived classes.
	*/
	ReportFrameLongTable(QtExt::Report* report, QTextDocument* textDocument);

	/*! Prepares frame for drawing.
		\param paintDevice Paint device in order to calculate correct sizes.
		\param width Maximum possible width for drawing.
	*/
	virtual void update(QPaintDevice* paintDevice, double width) override;

	/*! Calculates the number of possible subframes in case the frame can be divided.
		If the frame is indivisible it return 1.
		\param heightFirst Rest height of the first page
		\param heightRest Height of all other pages (whole page height).
	*/
	virtual unsigned int numberOfSubFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) const override;

	/*! Calculates the number of possible subframes in case the frame can be divided.
		If the frame is dividable it return a vector of subframes.
		If the frame is indivisible it return only the main frame.
		\param heightFirst Rest height of the first page
		\param heightRest Height of all other pages (normally whole page height).
	*/
	virtual std::vector<ReportFrameBase*> subFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) override;

	/*! Configure the internal table.
		Must be implemented by derived class.*/
	virtual void configureTable() = 0;

	/*! Configure the internal table.
		Must be implemented by derived class.*/
	virtual void configureHeading() = 0;

	QtExt::TextFrame	m_heading;			///< Text frame for heading.

	QtExt::Table		m_table;			///< Table for room data.

	qreal				m_headingSpace;		///< Space between heading and table.
	qreal				m_previousSpace;	///< Space before this frame.
	qreal				m_headingHeight;	///< Height of the heading
};

} // namespace QtExt {

#endif // QtExt_ReportFrameLongTableH
