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

#include "SVSimulationRunRequestDialog.h"
#include "ui_SVSimulationRunRequestDialog.h"

#include "SVSettings.h"

SVSimulationRunRequestDialog::SVSimulationRunRequestDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationRunRequestDialog)
{
	m_ui->setupUi(this);

	if (SVSettings::instance().m_propertyMap[SVSettings::PT_ClearResultDirBeforeStart].isValid() &&
			SVSettings::instance().m_propertyMap[SVSettings::PT_ClearResultDirBeforeStart].toBool() == true)
	{
		m_ui->checkBoxClearResultDir->setChecked(true);
	}
	else {
		m_ui->checkBoxClearResultDir->setChecked(false);
	}

	m_ui->pushButtonStart->setDefault(true);
}


SVSimulationRunRequestDialog::SimulationStartType SVSimulationRunRequestDialog::askForOption() {
	m_simulationStartType = DoNotRun;
	if (exec() == QDialog::Rejected)
		return DoNotRun;
	else
		return m_simulationStartType;
}


SVSimulationRunRequestDialog::~SVSimulationRunRequestDialog() {
	delete m_ui;
}


void SVSimulationRunRequestDialog::on_pushButtonStart_clicked() {
	m_simulationStartType = Normal;
	accept();
}


void SVSimulationRunRequestDialog::on_pushButtonContinue_clicked() {
	m_simulationStartType = Continue;
	accept();
}


void SVSimulationRunRequestDialog::on_checkBoxClearResultDir_toggled(bool checked) {
	SVSettings::instance().m_propertyMap[SVSettings::PT_ClearResultDirBeforeStart] = checked;
}
