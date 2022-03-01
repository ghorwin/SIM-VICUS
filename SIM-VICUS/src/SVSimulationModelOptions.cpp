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

#include "SVSimulationModelOptions.h"
#include "ui_SVSimulationModelOptions.h"

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Location.h>

SVSimulationModelOptions::SVSimulationModelOptions(QWidget *parent,
												   NANDRAD::SimulationParameter & simParams,
												   NANDRAD::Location & location) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationModelOptions),
	m_simParams(&simParams),
	m_location(&location)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	// populate combo box with solar distribution model options
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(true);
	for (int i=0; i<NANDRAD::SolarLoadsDistributionModel::NUM_SWR; ++i) {
		m_ui->comboBoxSolarDistributionModeltype->addItem(tr("%1 [%2]")
				.arg(NANDRAD::KeywordListQt::Description("SolarLoadsDistributionModel::distribution_t", i))
				.arg(NANDRAD::KeywordList::Keyword("SolarLoadsDistributionModel::distribution_t", i)));
	}
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(false);

	m_ui->lineEditInitialTemperature->setup(-50,150,tr("Initial temperature to be used for zones/constructions etc."), true, true);

}


SVSimulationModelOptions::~SVSimulationModelOptions() {
	delete m_ui;
}


void SVSimulationModelOptions::updateUi() {
	double initialTemperature = 20;
	if (!m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].name.empty() &&
		m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].IO_unit.base_id() == IBK::Unit("K").base_id())
	{
		initialTemperature = m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].get_value("C");
	}

	m_ui->lineEditInitialTemperature->setValue(initialTemperature);
	m_ui->checkBoxUsePerez->setChecked(m_location->m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].isEnabled());
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(true);
	m_ui->comboBoxSolarDistributionModeltype->setCurrentIndex(m_simParams->m_solarLoadsDistributionModel.m_distributionType);
	on_comboBoxSolarDistributionModeltype_currentIndexChanged(m_simParams->m_solarLoadsDistributionModel.m_distributionType);
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(false);

}



void SVSimulationModelOptions::on_comboBoxSolarDistributionModeltype_currentIndexChanged(int index) {
	/// TODO
	switch ((NANDRAD::SolarLoadsDistributionModel::distribution_t)index) {
		case NANDRAD::SolarLoadsDistributionModel::SWR_AreaWeighted:
		break;
		case NANDRAD::SolarLoadsDistributionModel::SWR_SurfaceTypeFactor:
		break;
		case NANDRAD::SolarLoadsDistributionModel::SWR_ViewFactor:
		break;
		case NANDRAD::SolarLoadsDistributionModel::NUM_SWR:
		break;
	}
}


void SVSimulationModelOptions::on_lineEditInitialTemperature_editingFinished() {
	if(m_ui->lineEditInitialTemperature->isValid()){
		double val = m_ui->lineEditInitialTemperature->value();
		NANDRAD::KeywordList::setParameter(m_simParams->m_para,
										   "SimulationParameter::para_t", NANDRAD::SimulationParameter::P_InitialTemperature, val);
	}
	else{
		double val = 20;
		if (!m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].name.empty() &&
			m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].IO_unit.base_id() == IBK::Unit("K").base_id())
		{
			val = m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].get_value("C");
		}
		m_ui->lineEditInitialTemperature->setValue(val);
	}
}

