#include "QtExt_ReportFrameTablePart.h"

#include "QtExt_ReportFrameItemTable.h"
#include "QtExt_ReportFrameItemTextFrame.h"
#include "QtExt_TextFrame.h"
#include "QtExt_Table.h"


namespace QtExt {

ReportFrameTablePart::ReportFrameTablePart(QtExt::Report* report, QTextDocument* textDocument, QtExt::TextFrame* heading, QtExt::Table* table, qreal headingSpace) :
	QtExt::ReportFrameBase(report, textDocument),
	m_heading(heading),
	m_table(table),
	m_headingSpace(headingSpace)
{}

ReportFrameTablePart::~ReportFrameTablePart() {
	delete m_heading;
	delete m_table;
}

void ReportFrameTablePart::update(QPaintDevice* paintDevice, double width) {

	if(m_heading != nullptr) {
		// set heading text
		addItem( new QtExt::ReportFrameItemTextFrame(m_heading, paintDevice, width, m_headingSpace));
	}

	if(m_table != nullptr)
		addItem( new QtExt::ReportFrameItemTable(m_table, paintDevice, width, m_headingSpace));

	QtExt::ReportFrameBase::update(paintDevice, width);
}

unsigned int ReportFrameTablePart::numberOfSubFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) const {
	return 1;
}

std::vector<QtExt::ReportFrameBase*> ReportFrameTablePart::subFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) {
	std::vector<QtExt::ReportFrameBase*> res;
	res.push_back(this);
	return res;
}

} // namespace QtExt {

