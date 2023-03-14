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

#include <SVProjectHandler.h>
#include <VICUS_Project.h>

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

	// values stored in same order as in NANDRAD::SimulationParameter::para_t
	std::vector<double> values {20, 50, 0, 0.1, 0.07, 20};
	std::vector<std::string> unitStr {"C", "%", "---", "1/h", "---", "C"};

	Q_ASSERT(values.size() == unitStr.size() && values.size() == NANDRAD::SimulationParameter::NUM_P);

	for(unsigned int i=0; i< NANDRAD::SimulationParameter::NUM_P; ++i){
		NANDRAD::SimulationParameter::para_t type = (NANDRAD::SimulationParameter::para_t)i;
		double &val = values[i];
		IBK::Unit unit(unitStr[i]);
		if (!m_simParams->m_para[type].name.empty() &&
				m_simParams->m_para[type].IO_unit.base_id() == unit.base_id()) {
			val = m_simParams->m_para[type].get_value(unit);
		}
	}

	/// ToDo werte noch setzen

	// now set all values
	m_ui->lineEditInitialTemperature->setValue(values[0]);

	std::vector<double> solarValues{10, 80, 10, 10};
	NANDRAD::SolarLoadsDistributionModel &solMod = m_simParams->m_solarLoadsDistributionModel;
	for(unsigned int i=0; i< solarValues.size(); ++i){
		NANDRAD::SolarLoadsDistributionModel::para_t type = (NANDRAD::SolarLoadsDistributionModel::para_t)i;
		double &val = solarValues[i];
		IBK::Unit unit("%");
		if (!solMod.m_para[type].name.empty() &&
				solMod.m_para[type].IO_unit.base_id() == unit.base_id()) {
			val = solMod.m_para[type].get_value(unit);
		}
		// prevent against inputting 0
		if (val < 0)
			val = 0;
	}

	// set percentages in spin boxes -> round to whole numbers!
	m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setValue((int)solarValues[0]);
	m_ui->spinBoxSolarRadiationToFloor->setValue((int)solarValues[1]);
	m_ui->spinBoxSolarRadiationToRoofCeiling->setValue((int)solarValues[2]);
	m_ui->spinBoxSolarRadiationToWalls->setValue((int)solarValues[3]);

	m_ui->checkBoxUsePerez->blockSignals(true);
	m_ui->checkBoxUsePerez->setChecked(m_location->m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].isEnabled());
	m_ui->checkBoxUsePerez->blockSignals(false);

	m_ui->checkBoxEnableMoistureBalance->blockSignals(true);
	m_ui->checkBoxEnableMoistureBalance->setChecked(m_simParams->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled());
	m_ui->checkBoxEnableMoistureBalance->blockSignals(false);

	m_ui->comboBoxSolarDistributionModeltype->blockSignals(true);
	int idx = m_simParams->m_solarLoadsDistributionModel.m_distributionType;
	m_ui->comboBoxSolarDistributionModeltype->setCurrentIndex(idx);
	on_comboBoxSolarDistributionModeltype_currentIndexChanged(idx);
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(false);

	m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setEnabled(idx == 0);

	m_ui->spinBoxSolarRadiationToFloor->setEnabled(idx == 1);
	m_ui->spinBoxSolarRadiationToRoofCeiling->setEnabled(idx == 1);
	m_ui->spinBoxSolarRadiationToWalls->setEnabled(idx == 1);


}



void SVSimulationModelOptions::on_comboBoxSolarDistributionModeltype_currentIndexChanged(int index) {
	NANDRAD::SolarLoadsDistributionModel &solMod = m_simParams->m_solarLoadsDistributionModel;
	solMod.m_distributionType = (NANDRAD::SolarLoadsDistributionModel::distribution_t)index;

	m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setEnabled(false);
	m_ui->spinBoxSolarRadiationToFloor->setEnabled(false);
	m_ui->spinBoxSolarRadiationToRoofCeiling->setEnabled(false);
	m_ui->spinBoxSolarRadiationToWalls->setEnabled(false);

	switch (solMod.m_distributionType) {
		case NANDRAD::SolarLoadsDistributionModel::SWR_AreaWeighted: {
			NANDRAD::SolarLoadsDistributionModel::para_t type = NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone;
			IBK::Unit unit("%");
			double val = 10;
			if (!solMod.m_para[type].name.empty() && solMod.m_para[type].IO_unit.base_id() == unit.base_id()) {
				val = solMod.m_para[type].get_value(unit);
			}
			else
				NANDRAD::KeywordList::setParameter(solMod.m_para, "SolarLoadsDistributionModel::para_t", type, val);
			m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setValue(val);
			m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setEnabled(true);
		}
		break;
		case NANDRAD::SolarLoadsDistributionModel::SWR_SurfaceTypeFactor: {

			std::vector<double>			values{80, 10, 10};
			std::vector<NANDRAD::SolarLoadsDistributionModel::para_t> types{
				NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionFloor,
				NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionCeiling,
				NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionWalls
			};
			double summedFractions = 0;
			for(unsigned int i=0; i<values.size(); ++i){
				NANDRAD::SolarLoadsDistributionModel::para_t type = (NANDRAD::SolarLoadsDistributionModel::para_t)(i+1);
				IBK::Unit unit("%");
				double val = 10;
				if (!solMod.m_para[type].name.empty() && solMod.m_para[type].IO_unit.base_id() == unit.base_id()) {
					val = solMod.m_para[type].get_value(unit);
				}
				else
					NANDRAD::KeywordList::setParameter(solMod.m_para, "SolarLoadsDistributionModel::para_t", type, val);
				summedFractions += val;
			}
			/// ToDo hier muss mal ne Fehlerabfrage rein und an den Nutzer durchgeschickt werden falls die Addition != 1 ist

			m_ui->spinBoxSolarRadiationToFloor->setValue(values[0]);
			m_ui->spinBoxSolarRadiationToRoofCeiling->setValue(values[1]);
			m_ui->spinBoxSolarRadiationToWalls->setValue(values[2]);

			m_ui->spinBoxSolarRadiationToFloor->setEnabled(true);
			m_ui->spinBoxSolarRadiationToRoofCeiling->setEnabled(true);
			m_ui->spinBoxSolarRadiationToWalls->setEnabled(true);
			//double val = 1- summedFractions;
			//val = std::max<double>(0, val);
			//m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setValue(val * 100);

		}
		break;
		case NANDRAD::SolarLoadsDistributionModel::SWR_ViewFactor: {
			/// ToDo ...

		}
		break;
		case NANDRAD::SolarLoadsDistributionModel::NUM_SWR:
		break;
	}
}


void SVSimulationModelOptions::on_lineEditInitialTemperature_editingFinished() {
	if(m_ui->lineEditInitialTemperature->isValid()){
		double val = m_ui->lineEditInitialTemperature->value();
		// we set the IBK Parameter
		m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature] = IBK::Parameter("InitialTemperature", val, IBK::Unit("C"));
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



void SVSimulationModelOptions::on_spinBoxSolarRadiationGainsDirectlyToRoomNode_valueChanged(int /*arg1*/) {
	double val = m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->value();
	m_simParams->m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone] = IBK::Parameter("RadiationLoadFractionZone", val, IBK::Unit("%"));
}


void SVSimulationModelOptions::on_spinBoxSolarRadiationToFloor_valueChanged(int /*arg1*/) {
	double val = m_ui->spinBoxSolarRadiationToFloor->value();
	m_simParams->m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionFloor] = IBK::Parameter("RadiationLoadFractionFloor", val, IBK::Unit("%"));
}


void SVSimulationModelOptions::on_spinBoxSolarRadiationToRoofCeiling_valueChanged(int /*arg1*/) {
	double val = m_ui->spinBoxSolarRadiationToRoofCeiling->value();
	m_simParams->m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionCeiling] = IBK::Parameter("RadiationLoadFractionCeiling", val, IBK::Unit("%"));
}


void SVSimulationModelOptions::on_spinBoxSolarRadiationToWalls_valueChanged(int /*arg1*/) {
	double val = m_ui->spinBoxSolarRadiationToWalls->value();
	m_simParams->m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionWalls] = IBK::Parameter("RadiationLoadFractionWalls", val, IBK::Unit("%"));
}


void SVSimulationModelOptions::on_checkBoxUsePerez_toggled(bool checked) {
	if (checked)
		m_location->m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].set( NANDRAD::KeywordList::Keyword("Location::flag_t", NANDRAD::Location::F_PerezDiffuseRadiationModel), true);
	else
		m_location->m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].clear();
}


void SVSimulationModelOptions::on_checkBoxEnableMoistureBalance_toggled(bool checked) {
	// set/clear flag
	if (checked)
		m_simParams->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].set(
					NANDRAD::KeywordList::Keyword("SimulationParameter::flag_t", NANDRAD::SimulationParameter::F_EnableMoistureBalance), true);
	else
		m_simParams->m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].clear();
}
