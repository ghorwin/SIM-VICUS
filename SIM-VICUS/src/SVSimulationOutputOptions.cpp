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

#include "SVSimulationOutputOptions.h"
#include "ui_SVSimulationOutputOptions.h"

#include <VICUS_Outputs.h>
#include <VICUS_KeywordList.h>

#include "SVStyle.h"

SVSimulationOutputOptions::SVSimulationOutputOptions(QWidget *parent, VICUS::Outputs & outputs) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationOutputOptions),
	m_outputs(&outputs)
{
	m_ui->setupUi(this);

	m_ui->tableWidgetOutputGrids->setColumnCount(4);
	m_ui->tableWidgetOutputGrids->setHorizontalHeaderLabels( QStringList() << tr("Name") << tr("Intervals") << tr("Start") << tr("End") );
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetOutputGrids);
	m_ui->tableWidgetOutputGrids->setSortingEnabled(false);
}


SVSimulationOutputOptions::~SVSimulationOutputOptions() {
	delete m_ui;
}


void SVSimulationOutputOptions::updateUi() {

	m_ui->checkBoxDefaultZoneOutputs->setChecked(
				m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].isEnabled());

	m_ui->checkBoxDefaultNetworkOutputs->setChecked(
				m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].isEnabled());

	m_ui->tableWidgetOutputGrids->clearContents();
	m_ui->tableWidgetOutputGrids->setRowCount(m_outputs->m_grids.size());
	for (unsigned int i=0; i<m_outputs->m_grids.size(); ++i) {
		const NANDRAD::OutputGrid & og = m_outputs->m_grids[i];
		QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(og.m_name));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetOutputGrids->setItem((int)i,0, item);


		// only populate start and end if intervals are all valid
		try {
			og.checkIntervalDefinition();

			item = new QTableWidgetItem(QString("%1").arg(og.m_intervals.size()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,1, item);

			QString start = QString("+ %L1 %2")
					.arg(og.m_intervals[0].m_para[NANDRAD::Interval::P_Start].value)
					.arg(QString::fromStdString(og.m_intervals[0].m_para[NANDRAD::Interval::P_Start].IO_unit.name()));

			item = new QTableWidgetItem(start);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,2, item);

			QString end;
			if (og.m_intervals.back().m_para[NANDRAD::Interval::P_End].value == 0.0)
				end = tr("End of simulation");
			else {
				end = QString("+ %L1 %2")
									.arg(og.m_intervals.back().m_para[NANDRAD::Interval::P_End].value)
									.arg(QString::fromStdString(og.m_intervals.back().m_para[NANDRAD::Interval::P_End].IO_unit.name()));
			}

			item = new QTableWidgetItem(end);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,3, item);

		}
		catch (...) {
			item = new QTableWidgetItem("---");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,1, item);
			item = new QTableWidgetItem("---");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,2, item);
			item = new QTableWidgetItem("---");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,3, item);
		}
	}

}


void SVSimulationOutputOptions::on_checkBoxDefaultZoneOutputs_toggled(bool checked) {
	if (checked)
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs]
				.set(VICUS::KeywordList::Keyword("Outputs::flag_t", VICUS::Outputs::F_CreateDefaultZoneOutputs), checked);
	else
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].clear();
}

void SVSimulationOutputOptions::on_checkBoxDefaultNetworkOutputs_toggled(bool checked){
	if (checked)
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs]
				.set(VICUS::KeywordList::Keyword("Outputs::flag_t", VICUS::Outputs::F_CreateDefaultNetworkOutputs), checked);
	else
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].clear();
}
