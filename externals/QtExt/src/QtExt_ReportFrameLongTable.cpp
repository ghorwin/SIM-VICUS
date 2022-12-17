#include "QtExt_ReportFrameLongTable.h"

#include "QtExt_ReportFrameItemTable.h"
#include "QtExt_ReportFrameItemTextFrame.h"
#include "QtExt_ReportFrameTablePart.h"
#include "QtExt_Report.h"


namespace QtExt {

ReportFrameLongTable::ReportFrameLongTable(QtExt::Report* report, QTextDocument* textDocument) :
	QtExt::ReportFrameBase(report, textDocument),
	m_heading(textDocument),
	m_table(textDocument, false),
	m_headingSpace(0),
	m_previousSpace(0),
	m_headingHeight(0)
{}

void ReportFrameLongTable::update(QPaintDevice* paintDevice, double width) {

	// set heading
	configureHeading();
	m_headingHeight = m_heading.frameRect(paintDevice, width).height();
	addItem( new QtExt::ReportFrameItemTextFrame(&m_heading, paintDevice, width, m_headingSpace));

	// set table
	configureTable();
	addItem( new QtExt::ReportFrameItemTable(&m_table, paintDevice, width, m_headingSpace));

	QtExt::ReportFrameBase::update(paintDevice, width);

}

unsigned int ReportFrameLongTable::numberOfSubFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) const {
	unsigned int tableCount = (unsigned int)m_table.fittingTableRows(paintDevice, heightFirst - m_headingHeight, heightRest).size();
	return tableCount;
}

std::vector<QtExt::ReportFrameBase*> ReportFrameLongTable::subFrames(QPaintDevice* paintDevice, double heightFirst, double heightRest) {
	int num = numberOfSubFrames(paintDevice, heightFirst, heightRest);
	std::vector<QtExt::Table*> subTables = m_table.fittingTables(paintDevice, heightFirst - m_headingHeight, heightRest);
	Q_ASSERT(num == subTables.size());
	Q_ASSERT(num > 0);
	clearSubFrames();
	m_currentSubFrames.push_back(new ReportFrameTablePart(m_report, m_textDocument, m_heading.clone(), subTables.front()));
	for(size_t i=1; i<subTables.size(); ++i) {
		m_currentSubFrames.push_back(new ReportFrameTablePart(m_report, m_textDocument, nullptr, subTables[i]));
	}

	return m_currentSubFrames;
}


} // namespace QtExt {
