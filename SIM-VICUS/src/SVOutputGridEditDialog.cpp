#include "SVOutputGridEditDialog.h"
#include "ui_SVOutputGridEditDialog.h"

#include <QMessageBox>

#include <NANDRAD_KeywordList.h>

#include <VICUS_Outputs.h>
#include <QtExt_Conversions.h>

SVOutputGridEditDialog::SVOutputGridEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVOutputGridEditDialog)
{
	m_ui->setupUi(this);
}


SVOutputGridEditDialog::~SVOutputGridEditDialog() {
	delete m_ui;
}


bool SVOutputGridEditDialog::edit(NANDRAD::OutputGrid & def, const VICUS::Outputs & outputs, int ownIndex) {
	m_outputs = &outputs;
	m_grid = &def;
	m_ownIdx = ownIndex;

	QString name = QString::fromStdString(def.m_name).trimmed();
	m_ui->lineEditName->setText( name );

	// create default interval if none is given yet (i.e. when we have created a new output grid)
	if (m_grid->m_intervals.empty()) {
		NANDRAD::Interval ival;
		ival.m_para[NANDRAD::Interval::P_Start].set(
					NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_Start), 0, IBK::Unit("d"));
		ival.m_para[NANDRAD::Interval::P_End].set(
					NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_End), 0, IBK::Unit("d"));
		ival.m_para[NANDRAD::Interval::P_StepSize].set(
					NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_StepSize), 1, IBK::Unit("h"));
		m_grid->m_intervals.push_back(ival);
	}

	// transfer data into user interface
	m_ui->spinBoxIntervalCount->blockSignals(true);
	m_ui->spinBoxIntervalCount->setValue((int)def.m_intervals.size());
	m_ui->spinBoxIntervalCount->blockSignals(false);
	updateIntervalTable();

	// *** execute dialog ***
	if (exec() != QDialog::Accepted)
		return false;

	// *** transfer data to condition, it has been checked already in accept() ***

	def = NANDRAD::OutputGrid(); // clear condition
	def.m_name = m_ui->lineEditName->text().toStdString();
	storeIntervals(def.m_intervals);

	return true;
}



void SVOutputGridEditDialog::accept() {
	if (m_ui->lineEditName->isEnabled()) {
		// check for valid input
		std::string name = m_ui->lineEditName->text().toStdString();
		if (name.empty()) {
			QMessageBox::critical(this, tr("Input error"),
								  tr("Please enter an ID name!"));
			return;
		}
		for (unsigned int i=0; i<m_outputs->m_grids.size(); ++i) {
			// skip own index on check
			if (i == (unsigned int)m_ownIdx)
				continue;
			if (m_outputs->m_grids[i].m_name == name) {
				QMessageBox::critical(this, tr("Input error"),
									  tr("The ID name '%1' exists already. Please select a different ID name!")
									  .arg(QString::fromStdString(name) ) );
				return;
			}
		}
	}

	if (!checkIntervals())
		return;

	QDialog::accept();
}


void SVOutputGridEditDialog::on_spinBoxIntervalCount_valueChanged(int newColumnCount) {
	unsigned int currentColumnCount = (unsigned int)m_ui->tableWidget->columnCount();
	m_ui->tableWidget->blockSignals(true);
	if (newColumnCount > m_ui->tableWidget->columnCount()) {
		// insert columns
		m_ui->tableWidget->setColumnCount(newColumnCount);
		std::vector<NANDRAD::Interval> * intervals = &m_grid->m_intervals;
		// and specify new parameters in columns unless original interval vector has
		// enough columns
		for (unsigned int i = currentColumnCount; i<(unsigned int)newColumnCount; ++i) {
			NANDRAD::Interval ival;
			// reuse existing interval definitions, if in original data structure
			if (intervals->size() > (unsigned int)i) {
				ival = (*intervals)[i];
			}
			else {
				// define default interval
				ival.m_para[NANDRAD::Interval::P_End].set(
							NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_End), 0, IBK::Unit("d"));
				ival.m_para[NANDRAD::Interval::P_StepSize].set(
							NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_StepSize), 1, IBK::Unit("h"));
			}
			fillColumn(i, ival);
		}
	}
	else {
		// remove columns
		m_ui->tableWidget->setColumnCount(newColumnCount);
	}
	m_ui->tableWidget->blockSignals(false);

	updateCalculatedIntervalTable();
}


void SVOutputGridEditDialog::updateIntervalTable() {
	// different table setup for schedule and output grid
	// output grids have additional row with step size input


	QStringList vHeaderLabels;
	vHeaderLabels << tr("Start time") << tr("Duration") << tr("End time") << tr("Step size");
	m_ui->tableWidget->blockSignals(true);
	m_ui->tableWidget->clear();
	m_ui->tableWidget->setRowCount(vHeaderLabels.count());
	m_ui->tableWidget->setColumnCount(m_ui->spinBoxIntervalCount->value());
	m_ui->tableWidget->setVerticalHeaderLabels(vHeaderLabels);

	std::vector<NANDRAD::Interval> * intervals = &m_grid->m_intervals;
	for (unsigned int i=0; i<intervals->size(); ++i) {
		fillColumn(i, (*intervals)[i]);
	}

	// now update the calculated table
	updateCalculatedIntervalTable();

	m_ui->tableWidget->blockSignals(false);
}


void SVOutputGridEditDialog::updateCalculatedIntervalTable() {

	QStringList vHeaderLabels;
	vHeaderLabels << tr("Start time") << tr("Duration") << tr("End time") << tr("Step size");
	m_ui->tableWidgetCalculated->clear();
	m_ui->tableWidgetCalculated->setRowCount(vHeaderLabels.count());
	m_ui->tableWidgetCalculated->setColumnCount(m_ui->spinBoxIntervalCount->value());
	m_ui->tableWidgetCalculated->setVerticalHeaderLabels(vHeaderLabels);

	for (int i=0; i<m_ui->spinBoxIntervalCount->value(); ++i) {
		QString label = tr("Interval #%1").arg(i+1);
		m_ui->tableWidgetCalculated->setHorizontalHeaderItem(i, new QTableWidgetItem(label));
	}

	std::vector<NANDRAD::Interval> intervals;
	parseTable(intervals, false); // parses table and populate grid data structure

	// now fill in all table widget items
	IBK::Parameter lastEndTime("last end time", 0, IBK::Unit("d"));
	for (unsigned int i=0; i<intervals.size(); ++i) {
		NANDRAD::Interval & ival = intervals[i];
		// if value is given in interval, transfer it to user interface
		QTableWidgetItem * item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled);
		if (!ival.m_para[NANDRAD::Interval::P_Start].name.empty()) {
			item->setText( QString::fromStdString(ival.m_para[NANDRAD::Interval::P_Start].toString()) );
			lastEndTime = ival.m_para[NANDRAD::Interval::P_Start];
		}
		else {
			// no start value, use last end time, if it is set
			item->setText( QString::fromStdString(lastEndTime.toString()) );
			QFont f;
			f.setItalic(true);
			item->setFont(f);
			item->setTextColor(Qt::gray);
			ival.m_para[NANDRAD::Interval::P_Start] = lastEndTime;
		}
		m_ui->tableWidgetCalculated->setItem(0, i, item);

		// now ival contains a valid start time entry

		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled);
		// calculate from end time
		double duration = ival.m_para[NANDRAD::Interval::P_End].value - ival.m_para[NANDRAD::Interval::P_Start].value;
		IBK::Parameter dur("Duration", duration);
		dur.IO_unit = ival.m_para[NANDRAD::Interval::P_End].IO_unit;

		item->setText( QString::fromStdString(dur.toString()) );
		QFont f;
		f.setItalic(true);
		item->setFont(f);
		item->setTextColor(Qt::gray);
		m_ui->tableWidgetCalculated->setItem(1, i, item);

		// end time
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled);
		item->setText( QString::fromStdString(ival.m_para[NANDRAD::Interval::P_End].toString()) );
		lastEndTime = ival.m_para[NANDRAD::Interval::P_End];
		m_ui->tableWidgetCalculated->setItem(2, i, item);

		// end time
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled);
		item->setText( QString::fromStdString(ival.m_para[NANDRAD::Interval::P_StepSize].toString()) );
		m_ui->tableWidgetCalculated->setItem(3, i, item);
	}
}


void SVOutputGridEditDialog::fillColumn(int columnIdx, const NANDRAD::Interval & ival) {
	QString label = tr("Interval #%1").arg(columnIdx+1);
	m_ui->tableWidget->setHorizontalHeaderItem(columnIdx, new QTableWidgetItem(label));

	// start time
	QTableWidgetItem * item = new QTableWidgetItem;
	// in output grids, only the first column's start time can be edited
	if (columnIdx != 0) {
		item->setFlags(Qt::NoItemFlags);
		item->setBackgroundColor(QPalette().color(QPalette::Shadow));
	}
	else {
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		if (!ival.m_para[NANDRAD::Interval::P_Start].name.empty())
			item->setText( QString::fromStdString(ival.m_para[NANDRAD::Interval::P_Start].toString()) );
	}
	m_ui->tableWidget->setItem(0, columnIdx, item);

	item = new QTableWidgetItem;
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	m_ui->tableWidget->setItem(1,columnIdx, item);

	item = new QTableWidgetItem;
	if (!ival.m_para[NANDRAD::Interval::P_End].name.empty())
		item->setText( QString::fromStdString(ival.m_para[NANDRAD::Interval::P_End].toString()) );
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	m_ui->tableWidget->setItem(2,columnIdx, item);

	item = new QTableWidgetItem;
	if (!ival.m_para[NANDRAD::Interval::P_StepSize].name.empty())
		item->setText( QString::fromStdString(ival.m_para[NANDRAD::Interval::P_StepSize].toString()) );
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	m_ui->tableWidget->setItem(3,columnIdx, item);
}


bool SVOutputGridEditDialog::checkIntervals() {
	std::vector<NANDRAD::Interval> intervals;
	if (!parseTable(intervals, true))
		return false;
	return true;
}


void SVOutputGridEditDialog::storeIntervals(std::vector<NANDRAD::Interval> & intervals) const {
	int success = parseTable(intervals, false);
	Q_ASSERT(success);
}


bool SVOutputGridEditDialog::parseTable(std::vector<NANDRAD::Interval> & intervals, bool showMessageOnError) const {
	// parse the values in the table and populate the data structure

	intervals.clear();
	bool success = true;
#if 0
	for (int i=0; i<m_ui->spinBoxIntervalCount->value(); ++i) {
		NANDRAD::Interval ival;
		ival.m_para[NANDRAD::Interval::IP_START].clear();
		// if in output grid mode, only take start parameter from first interval
		if (i == 0 || m_isSchedule) {
			if (!QString2Parameter(m_ui->tableWidget->item(0,i)->text(),
								   NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::IP_START),
								   ival.m_para[NANDRAD::Interval::IP_START]))
			{
				if (showMessageOnError)
					showError(0,i,tr("Invalid start time."));
				return false;
			}
		}

		// populate all other parameters

		// duration
		QString valStr = m_ui->tableWidget->item(1,i)->text().trimmed();

		if (!valStr.isEmpty() && !QString2Parameter(valStr,
							   NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::IP_DURATION),
							   ival.m_para[NANDRAD::Interval::IP_DURATION]))
		{
			if (showMessageOnError)
				showError(1,i,tr("Invalid duration."));
			return false;
		}


		// end time
		valStr = m_ui->tableWidget->item(2,i)->text().trimmed();

		if (!valStr.isEmpty() && !QString2Parameter(valStr,
							   NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::IP_END),
							   ival.m_para[NANDRAD::Interval::IP_END]))
		{
			if (showMessageOnError)
				showError(2,i,tr("Invalid end time."));
			return false;
		}

		// step size - only in output grid mode
		if (!m_isSchedule) {

			valStr = m_ui->tableWidget->item(3,i)->text().trimmed();

			if (valStr.isEmpty()) {
				if (showMessageOnError)
					showError(3,i,tr("Missing step size."));
				return false;
			}
			else if (!QString2Parameter(valStr,
										NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::IP_STEPSIZE),
										ival.m_para[NANDRAD::Interval::IP_STEPSIZE]))

			{
				if (showMessageOnError)
					showError(3,i,tr("Invalid step size."));
				return false;
			}

			if (ival.m_para[NANDRAD::Interval::IP_STEPSIZE].value <=0) {
				if (showMessageOnError)
					showError(3,i,tr("Invalid step size."));
				return false;
			}
		}

		// check for missing end time or duration, or duplicate definition
		if (!ival.m_para[NANDRAD::Interval::IP_END].name.empty() &&
			!ival.m_para[NANDRAD::Interval::IP_DURATION].name.empty()) {
			if (showMessageOnError)
				showError(1,i,tr("Redundant definition of end time and duration is not allowed."));
			return false;
		}

		if (ival.m_para[NANDRAD::Interval::IP_END].name.empty() &&
			ival.m_para[NANDRAD::Interval::IP_DURATION].name.empty())
		{
			if (showMessageOnError)
				showError(1,i,tr("Either duration or end time is required."));
			return false;
		}

		// check against negative duration
		if (!ival.m_para[NANDRAD::Interval::IP_DURATION].name.empty()) {
			if (ival.m_para[NANDRAD::Interval::IP_DURATION].value < 0) {
				if (showMessageOnError)
					showError(1,i,tr("Negative duration is not allowed."));
				return false;
			}

			// check against zero duration in earlier interval
			if (ival.m_para[NANDRAD::Interval::IP_DURATION].value == 0 &&
				i+1 != m_ui->spinBoxIntervalCount->value())
			{
				if (showMessageOnError)
					showError(1,i,tr("A duration of zero is only allowed in the last interval."));
				return false;
			}
		}

		// interval ok, append to list of intervals
		intervals.push_back(ival);
	}
#endif
	return success;
}


void SVOutputGridEditDialog::showError(int row, int col, const QString & text) const {
	m_ui->tableWidget->item(row, col)->setSelected(true);
	m_ui->tableWidget->setFocus();
	QMessageBox::critical(const_cast<SVOutputGridEditDialog*>(this), tr("Invalid input"), text);
}


void SVOutputGridEditDialog::on_tableWidget_cellChanged(int row, int column) {
	// once a valid duration has been entered, clear the corresponding end time and vice versa
	if (row == 1) {
		IBK::Parameter p;
		if (QtExt::QString2Parameter(m_ui->tableWidget->item(row, column)->text(), "dur", p)) {
			m_ui->tableWidget->blockSignals(true);
			m_ui->tableWidget->item(2, column)->setText("");
			m_ui->tableWidget->blockSignals(false);
		}
	}
	if (row == 2) {
		IBK::Parameter p;
		if (QtExt::QString2Parameter(m_ui->tableWidget->item(row, column)->text(), "et", p)) {
			m_ui->tableWidget->blockSignals(true);
			m_ui->tableWidget->item(1, column)->setText("");
			m_ui->tableWidget->blockSignals(false);
		}
	}
	// update calculated table
	updateCalculatedIntervalTable();
}
