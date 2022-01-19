#include "SVOutputGridEditDialog.h"
#include "ui_SVOutputGridEditDialog.h"

#include <QMessageBox>

#include <NANDRAD_KeywordList.h>

#include <VICUS_Outputs.h>
#include <SV_Conversions.h>

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
			fillColumn((int)i, ival);
		}
	}
	else {
		// remove columns
		m_ui->tableWidget->setColumnCount(newColumnCount);
	}
	m_ui->tableWidget->blockSignals(false);
}


void SVOutputGridEditDialog::updateIntervalTable() {
	// different table setup for schedule and output grid
	// output grids have additional row with step size input


	QStringList vHeaderLabels;
	vHeaderLabels << tr("Start time") << tr("End time") << tr("Step size");
	m_ui->tableWidget->blockSignals(true);
	m_ui->tableWidget->clear();
	m_ui->tableWidget->setRowCount(vHeaderLabels.count());
	m_ui->tableWidget->setColumnCount(m_ui->spinBoxIntervalCount->value());
	m_ui->tableWidget->setVerticalHeaderLabels(vHeaderLabels);

	std::vector<NANDRAD::Interval> * intervals = &m_grid->m_intervals;
	for (unsigned int i=0; i<intervals->size(); ++i)
		fillColumn((int)i, (*intervals)[i]);

	m_ui->tableWidget->blockSignals(false);
}


void SVOutputGridEditDialog::fillColumn(int columnIdx, const NANDRAD::Interval & ival) {
	QString label = tr("Interval #%1").arg(columnIdx+1);
	m_ui->tableWidget->setHorizontalHeaderItem(columnIdx, new QTableWidgetItem(label));

	// start time
	QTableWidgetItem * item = new QTableWidgetItem;
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	if (!ival.m_para[NANDRAD::Interval::P_Start].name.empty())
		item->setText( QtExt::parameter2String(ival.m_para[NANDRAD::Interval::P_Start]) );
	m_ui->tableWidget->setItem(0, columnIdx, item);

	item = new QTableWidgetItem;
	// only show given end time points and time points that are not 0
	if (!ival.m_para[NANDRAD::Interval::P_End].name.empty() && ival.m_para[NANDRAD::Interval::P_End].value != 0.)
		item->setText( QtExt::parameter2String(ival.m_para[NANDRAD::Interval::P_End]) );
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	m_ui->tableWidget->setItem(1,columnIdx, item);

	item = new QTableWidgetItem;
	if (!ival.m_para[NANDRAD::Interval::P_StepSize].name.empty())
		item->setText( QtExt::parameter2String(ival.m_para[NANDRAD::Interval::P_StepSize]) );
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	m_ui->tableWidget->setItem(2,columnIdx, item);
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

	// start time and step size is mandatory
	// end time points are optional, and are only checked if specified

	for (int i=0; i<m_ui->spinBoxIntervalCount->value(); ++i) {
		NANDRAD::Interval ival;
		// extract start parameter
		QString valStr = m_ui->tableWidget->item(0,i)->text().trimmed();
		if (valStr.isEmpty()) {
			if (showMessageOnError)
				showError(0,i,tr("Missing interval start time."));
			return false;
		}
		if (!QtExt::QString2Parameter(m_ui->tableWidget->item(0,i)->text(),
			NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_Start),
			ival.m_para[NANDRAD::Interval::P_Start]))
		{
			if (showMessageOnError)
				showError(0,i,tr("Invalid start time."));
			return false;
		}

		// end time
		valStr = m_ui->tableWidget->item(1,i)->text().trimmed();

		if (!valStr.isEmpty() && !QtExt::QString2Parameter(valStr,
			NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_End),
			ival.m_para[NANDRAD::Interval::P_End]))
		{
			if (showMessageOnError)
				showError(1,i,tr("Invalid end time."));
			return false;
		}

		// step size
		valStr = m_ui->tableWidget->item(2,i)->text().trimmed();

		if (valStr.isEmpty()) {
			if (showMessageOnError)
				showError(2,i,tr("Missing step size."));
			return false;
		}
		else if (!QtExt::QString2Parameter(valStr,
			NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_StepSize),
			ival.m_para[NANDRAD::Interval::P_StepSize]))
		{
			if (showMessageOnError)
				showError(2,i,tr("Invalid step size."));
			return false;
		}

		if (ival.m_para[NANDRAD::Interval::P_StepSize].value <=0) {
			if (showMessageOnError)
				showError(2,i,tr("Invalid step size, must be > 0."));
			return false;
		}

		// protect against negative interval lengths
		if (!ival.m_para[NANDRAD::Interval::P_End].name.empty()) {
			if (ival.m_para[NANDRAD::Interval::P_End].value < ival.m_para[NANDRAD::Interval::P_Start].value) {
				if (showMessageOnError)
					showError(1,i,tr("Invalid interval definition, start point must preceed end point."));
				return false;
			}
		}

		// append to list of intervals
		intervals.push_back(ival);
	}

	// finally check that end time points never exceed next interval's start time points
	for (unsigned int i=1; i<intervals.size(); ++i) {
		// do we have an end point in the previous interval?
		if (!intervals[i-1].m_para[NANDRAD::Interval::P_End].name.empty()) {
			if (intervals[i-1].m_para[NANDRAD::Interval::P_End].value > intervals[i].m_para[NANDRAD::Interval::P_Start].value) {
				if (showMessageOnError)
					showError(0,(int)i,tr("Invalid interval definition, start point not preceed last interval's end point."));
				return false;
			}
		}
	}

	return success;
}


void SVOutputGridEditDialog::showError(int row, int col, const QString & text) const {
	m_ui->tableWidget->item(row, col)->setSelected(true);
	m_ui->tableWidget->setFocus();
	QMessageBox::critical(const_cast<SVOutputGridEditDialog*>(this), tr("Invalid input"), text);
}


void SVOutputGridEditDialog::on_tableWidget_cellChanged(int row, int column) {
//	IBK::Parameter p;
//	// if a valid start time has been entered and the previous interval's end edit is empty, update
//	// the "autotext" properties
//	if (QtExt::QString2Parameter(m_ui->tableWidget->item(row, column)->text(), "start", p) && row == 0) {
//		if (column > 0 && m_ui->tableWidget->item(1,column-1)->text().isEmpty()) {
//			QLineEdit * le = qobject_cast<QLineEdit *>(m_ui->tableWidget->cellWidget(2,column-1));
//			if (le == nullptr) {
//				le = new QLineEdit(m_ui->tableWidget);
//				m_ui->tableWidget->setCellWidget(2,column-1,le);
//			}
//			le->setPlaceholderText(QString::fromStdString(p.toString()));
//		}
//	}

}
