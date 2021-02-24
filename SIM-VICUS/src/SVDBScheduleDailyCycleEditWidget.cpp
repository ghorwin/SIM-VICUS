#include "SVDBScheduleDailyCycleEditWidget.h"
#include "ui_SVDBScheduleDailyCycleEditWidget.h"

#include <VICUS_ScheduleInterval.h>

#include <QItemDelegate>

#include "SVDatabase.h"
#include "SVStyle.h"

SVDBScheduleDailyCycleEditWidget::SVDBScheduleDailyCycleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleDailyCycleEditWidget){
	m_ui->setupUi(this);


	//add header to day cycle table
	m_ui->tableWidgetDayCycle->setColumnCount(2);
	m_ui->tableWidgetDayCycle->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Time")));
	m_ui->tableWidgetDayCycle->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Value")));

	//set all table items in day cycle
	m_ui->tableWidgetDayCycle->setRowCount(24);

	m_ui->tableWidgetDayCycle->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui->tableWidgetDayCycle->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_delegate = new QItemDelegate();
	m_ui->tableWidgetDayCycle->blockSignals(true);
	for(unsigned int i=0; i<24; ++i){
		QString time = i < 10 ? "0" + QString::number(i) : QString::number(i);
		time += ":00 - ";
		time += i+1 < 10 ? "0" + QString::number(i+1) : QString::number(i+1);
		time += ":00";

		m_ui->tableWidgetDayCycle->setItem(i,0, new QTableWidgetItem(time));
		m_ui->tableWidgetDayCycle->item(i,0)->setFlags(m_ui->tableWidgetDayCycle->item(i,0)->flags() & ~Qt::ItemIsEditable);


		QTableWidgetItem *item = new QTableWidgetItem("0");
		m_ui->tableWidgetDayCycle->setItem(i,1, item);
	}
	m_ui->tableWidgetDayCycle->blockSignals(false);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetDayCycle);
	m_ui->tableWidgetDayCycle->setSortingEnabled(false);

}

SVDBScheduleDailyCycleEditWidget::~SVDBScheduleDailyCycleEditWidget(){
	delete m_ui;
}



void SVDBScheduleDailyCycleEditWidget::updateInput(VICUS::DailyCycle *dc, SVDatabase *db){

	//timepoints in seconds
	//set values
	m_dailyCycle = dc;
	m_db = db;

	if(m_dailyCycle->m_timePoints.empty()){
		m_dailyCycle->m_timePoints.push_back(0);
		m_dailyCycle->m_values.push_back(0);
		m_db->m_schedules.m_modified;
	}
	m_ui->tableWidgetDayCycle->blockSignals(true);
	//check time point in schedule interval and write the table values
	for (unsigned int j=0;j<24 ; ++j) {
		double currTP = j/**3600*/;
		double val=0;
		for (unsigned int i=0; i<dc->m_timePoints.size(); ++i) {
			double intervalTp = dc->m_timePoints[i];
			if(currTP>=intervalTp)
				val=dc->m_values[i];
			else
				break;
		}
		m_ui->tableWidgetDayCycle->item(j, 1)->setText(QString::number(val,'g', 3));
	}
	m_ui->tableWidgetDayCycle->blockSignals(false);
}

void SVDBScheduleDailyCycleEditWidget::on_tableWidgetDayCycle_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn) {


}

void SVDBScheduleDailyCycleEditWidget::on_tableWidgetDayCycle_cellChanged(int row, int column)
{
	if(m_dailyCycle == nullptr)
		return;

	//validate input
	bool ok;
	m_ui->tableWidgetDayCycle->item(row,1)->text().toDouble(&ok);
	if(!ok){
		m_ui->tableWidgetDayCycle->blockSignals(true);
		//m_ui->tableWidgetDayCycle->item(row,1)->setText("0");
		m_ui->tableWidgetDayCycle->setFocus();
		m_ui->tableWidgetDayCycle->setCurrentCell(row,1);
		m_ui->tableWidgetDayCycle->blockSignals(false);
		QMessageBox::critical(this, QString(), tr("Wrong input in cell at row %1. Only values are allowed.").arg(row));
		return;
	}

//	//create a timeval element and a vector
//	struct timeVal{
//		double m_timepoint = 0;
//		double m_val = 0;
//	};

//	std::vector<timeVal> timeVals;
//	//import the table view
//	for(unsigned int i=0; i<m_ui->tableWidgetDayCycle->rowCount();++i){
//		timeVal tV;
//		tV.m_timepoint = i*3600;
//		tV.m_val = m_ui->tableWidgetDayCycle->item(i,1)->text().toDouble();

//		if(i==0 || !IBK::near_equal(timeVals.back().m_val,tV.m_val))
//			timeVals.push_back(tV);

//	}

//	//delete redundant objects
//	QStringList strL;
//	for (auto tv : timeVals ) {
//		strL << QString::number(tv.m_timepoint) + " | " + QString::number(tv.m_val);
//	}



	std::vector<double> timepoints(1,0), values(1, m_ui->tableWidgetDayCycle->item(0,1)->text().toDouble());
	unsigned int lastIdx=0;
	for(unsigned int i=1; i<m_ui->tableWidgetDayCycle->rowCount(); ++i){
		double currVal = m_ui->tableWidgetDayCycle->item(i,1)->text().toDouble();
		if(IBK::nearly_equal<3>(values[lastIdx],currVal))
			continue;
		timepoints.push_back(i/**3600*/);
		values.push_back(currVal);
		++lastIdx;
	}

	m_dailyCycle->m_timePoints.swap(timepoints);
	m_dailyCycle->m_values.swap(values);

	//set database to modified
	m_db->m_schedules.m_modified;
}
