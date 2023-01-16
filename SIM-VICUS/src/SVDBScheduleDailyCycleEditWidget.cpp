/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVDBScheduleDailyCycleEditWidget.h"
#include "ui_SVDBScheduleDailyCycleEditWidget.h"

#include <VICUS_ScheduleInterval.h>

#include <QItemDelegate>

#include <QtExt_Locale.h>

#include "SVDatabase.h"
#include "SVStyle.h"

SVDBScheduleDailyCycleEditWidget::SVDBScheduleDailyCycleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleDailyCycleEditWidget){
	m_ui->setupUi(this);

	// add header to day cycle table
	m_ui->tableWidgetDayCycle->setColumnCount(2);
	m_ui->tableWidgetDayCycle->setHorizontalHeaderLabels(QStringList() << tr("Time") << tr("Value") );

	// set all table items in day cycle
	m_ui->tableWidgetDayCycle->setRowCount(24);

	m_delegate = new QItemDelegate();
	m_ui->tableWidgetDayCycle->blockSignals(true);
	for (int i=0; i<24; ++i){
		QString time = QString("%1:00").arg(i,2,10,QChar('0'));

		QTableWidgetItem * item = new QTableWidgetItem(time);
		m_ui->tableWidgetDayCycle->setItem(i,0, item);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setTextAlignment(Qt::AlignCenter);

		item = new QTableWidgetItem();
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		m_ui->tableWidgetDayCycle->setItem(i,1, item);
		// value column is populated in updateUi()
	}
	m_ui->tableWidgetDayCycle->blockSignals(false);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetDayCycle);
	m_ui->tableWidgetDayCycle->setSortingEnabled(false);
	m_ui->tableWidgetDayCycle->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui->tableWidgetDayCycle->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// set period table column sizes
	int width = m_ui->tableWidgetDayCycle->width();

	m_ui->tableWidgetDayCycle->setColumnWidth(0, 100);
	m_ui->tableWidgetDayCycle->setColumnWidth(1, width-100);
}


SVDBScheduleDailyCycleEditWidget::~SVDBScheduleDailyCycleEditWidget(){
	delete m_ui;
}


void SVDBScheduleDailyCycleEditWidget::updateInput(VICUS::DailyCycle *dc, SVDatabase *db, bool isEditable){
	m_dailyCycle = dc;
	m_db = db;
	m_isEditable = isEditable;
	m_ui->tableWidgetDayCycle->clearSelection();

	// if dc == nullptr, clear inputs and return
	if (dc == nullptr) {
		m_ui->tableWidgetDayCycle->blockSignals(true);
		for (int i=0; i<24; ++i) {
			m_ui->tableWidgetDayCycle->item(i, 1)->setText("");
		}
		m_ui->tableWidgetDayCycle->blockSignals(false);
		return;
	}

	// if daily cycle is still empty, add a default value
	if (m_dailyCycle->m_timePoints.empty()) {
		m_dailyCycle->m_timePoints.push_back(0); // timepoints in seconds
		m_dailyCycle->m_values.push_back(0);
		modelModify();
	}

	// fill in table and widget
	updateTable(true);

	m_ui->tableWidgetDayCycle->setEnabled(m_isEditable);
	m_ui->dailyCycleInputWidget->setEnabled(m_isEditable);
	m_ui->labelEditInfo->setEnabled(m_isEditable);
}


void SVDBScheduleDailyCycleEditWidget::on_tableWidgetDayCycle_cellChanged(int row, int /*column*/) {
	if (m_dailyCycle == nullptr)
		return;

	// validate input
	bool ok;
	double currVal = QtExt::Locale().toDoubleWithFallback(m_ui->tableWidgetDayCycle->item(row,1)->text(), &ok);
	if (!ok){
		m_ui->tableWidgetDayCycle->blockSignals(true);
		m_ui->tableWidgetDayCycle->setFocus();
		m_ui->tableWidgetDayCycle->setCurrentCell(row,1);
		m_ui->tableWidgetDayCycle->blockSignals(false);
		QMessageBox::critical(this, QString(), tr("Wrong input in cell at row %1. Only values are allowed.").arg(row));
		return;
	}

	// we set the value to all rows
	QList<QTableWidgetSelectionRange> range = m_ui->tableWidgetDayCycle->selectedRanges();

	for (int i=0; i<range.size(); ++i) {
		// can we have two ranges?
		int topRow = range[i].topRow();
		int bottomRow = range[i].bottomRow()+1;

		for (int j=topRow; j<bottomRow; ++j) {
			m_ui->tableWidgetDayCycle->blockSignals(true);
			m_ui->tableWidgetDayCycle->item(j,1)->setText(QString("%L1").arg(currVal) );
			m_ui->tableWidgetDayCycle->blockSignals(false);
		}
	}

	double val = QtExt::Locale().toDoubleWithFallback(m_ui->tableWidgetDayCycle->item(0,1)->text(), &ok);
	// add first value and time "0 h" to vector
	std::vector<double> timepoints(1,0);
	std::vector<double> values(1, val);
	unsigned int lastIdx=0;

	// process all subsequent values but only add those that differ in value
	for (int i=1; i<m_ui->tableWidgetDayCycle->rowCount(); ++i) {
		val = QtExt::Locale().toDoubleWithFallback(m_ui->tableWidgetDayCycle->item(i,1)->text(), &ok);
		// TODO Dirk: check accuracy should be related to nominal quantity magnitude
		if (IBK::nearly_equal<3>(values[lastIdx],val))
			continue;
		timepoints.push_back(i); // in hours
		values.push_back(val);
		++lastIdx;
	}

	m_dailyCycle->m_timePoints.swap(timepoints);
	m_dailyCycle->m_values.swap(values);

	// update preview
	updateTable(true);
	//set database to modified
	modelModify();
}


void SVDBScheduleDailyCycleEditWidget::modelModify() {
	m_db->m_schedules.m_modified = true;
	emit dataChanged();
}


void SVDBScheduleDailyCycleEditWidget::on_dailyCycleInputWidget_valueChanged() {
	// get values from widget
	std::vector<double> vals = m_ui->dailyCycleInputWidget->values();
	// process all 24 values and whenever the value changes, insert a new time point and value pair
	std::vector<double> tp, data;
	tp.push_back(0);
	data.push_back(vals[0]);
	for (unsigned int i=1; i<24; ++i) {
		if (vals[i] == data.back())
			continue;
		tp.push_back(i);
		data.push_back(vals[i]);
	}
	m_dailyCycle->m_timePoints.swap(tp);
	m_dailyCycle->m_values.swap(data);

	// fill in table
	updateTable(false);
	modelModify();
}


void SVDBScheduleDailyCycleEditWidget::updateTable(bool updateDailyCycleWidget) {
	std::vector<double> hourlyValues;
	m_ui->tableWidgetDayCycle->blockSignals(true);
	// check time point in schedule interval and write the table values
	for (int j=0; j<24; ++j) {
		double currTP = j/**3600*/;
		double val=0;
		for (unsigned int i=0; i<m_dailyCycle->m_timePoints.size(); ++i) {
			double intervalTp = m_dailyCycle->m_timePoints[i];
			if (currTP>=intervalTp)
				val=m_dailyCycle->m_values[i];
			else
				break;
		}
		hourlyValues.push_back(val);
		// TODO Dirk: value formatting should/may depend on quantity being edited
		m_ui->tableWidgetDayCycle->item(j, 1)->setText(QString("%L1").arg(val, 0, 'g', 5));
		m_ui->tableWidgetDayCycle->item(j, 1)->setTextAlignment(Qt::AlignCenter);

		if (m_isEditable)
			m_ui->tableWidgetDayCycle->item(j,1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		else
			m_ui->tableWidgetDayCycle->item(j,1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
	m_ui->tableWidgetDayCycle->blockSignals(false);

	if (updateDailyCycleWidget)
		m_ui->dailyCycleInputWidget->setValues(hourlyValues);
	// TODO Dirk: adjust min/max values based on edited quantity
}
