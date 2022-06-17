#ifndef ReportFrameTablePartH
#define ReportFrameTablePartH


#include <QApplication>

#include "QtExt_ReportFrameBase.h"

namespace QtExt {

class Table;
class TextFrame;

/*! Frmae which can contain a header and a table. Both must be set outside.
	The class takes ownership of both items.
	Can be used for spliiting large tables (\see ReportFrameLongTable).*/
class ReportFrameTablePart : public ReportFrameBase {
	Q_DECLARE_TR_FUNCTIONS(ReportFrameTablePart)
	Q_DISABLE_COPY(ReportFrameTablePart)
public:
	/*! Only constructor.
		\param report Reference to report (owner).
		\param textDocument Current textDocument responsible for rendering text. Necessary for derived classes.
		\param heading Possible heading. Set nullptr in case of no heading exist. Class takes ownership.
		\param table Subtable to be shown. Class takes ownership
	*/
	ReportFrameTablePart(QtExt::Report* report, QTextDocument* textDocument, QtExt::TextFrame* heading, QtExt::Table* table, qreal headingSpace = 0);

	/*! Destructor deletes table and header.*/
	~ReportFrameTablePart();

	/*! Prepares frame for drawing.
		\param paintDevice Paint device in order to calculate correct sizes.
		\param width Maximum possible width for drawing.
	*/
	void update(QPaintDevice* paintDevice, double width) override;

	/*! Calculates the number of possible subframes in case the frame can be divided.
		If the frame is indivisible it return 1.
		\param heightFirst Rest height of the first page
		\param heightRest Height of all other pages (whole page height).
	*/
	unsigned int numberOfSubFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) const override;

	/*! Calculates the number of possible subframes in case the frame can be divided.
		If the frame is dividable it return a vector of subframes.
		If the frame is indivisible it return only the main frame.
		\param heightFirst Rest height of the first page
		\param heightRest Height of all other pages (normally whole page height).
	*/
	std::vector<ReportFrameBase*> subFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) override;

	TextFrame*	m_heading;			///< Text frame for heading.

	Table*		m_table;			///< Table.

	qreal		m_headingSpace;		///< Space between heading and table.
};

} // namespace QtExt {

#endif // ReportFrameTablePartH
