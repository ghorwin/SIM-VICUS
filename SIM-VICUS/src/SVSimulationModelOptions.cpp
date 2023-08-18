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

#include "SVProjectHandler.h"
#include "SVUndoModifySolverParameter.h"
#include "SVUndoModifyClimate.h"
#include "SVMainWindow.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"
#include "SVView3DDialog.h"

#include <VICUS_Project.h>

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_Location.h>

SVSimulationModelOptions::SVSimulationModelOptions(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationModelOptions)
{
	m_ui->setupUi(this);
	layout()->setContentsMargins(0,0,0,0);

	// populate combo box with solar distribution model options
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(true);
	for (int i=0; i<NANDRAD::SolarLoadsDistributionModel::NUM_SWR; ++i) {
		m_ui->comboBoxSolarDistributionModeltype->addItem(tr("%1 [%2]")
				.arg(NANDRAD::KeywordListQt::Description("SolarLoadsDistributionModel::distribution_t", i))
				.arg(NANDRAD::KeywordList::Keyword("SolarLoadsDistributionModel::distribution_t", i)));
	}
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(false);

	m_ui->lineEditInitialTemperature->setup(-50,150,tr("Initial temperature to be used for zones/constructions etc."), true, true);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVSimulationModelOptions::onModified);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			this, &SVSimulationModelOptions::onStyleChanged);

	onStyleChanged();
}


SVSimulationModelOptions::~SVSimulationModelOptions() {
	delete m_ui;
}


void SVSimulationModelOptions::updateUi() {

	const NANDRAD::SimulationParameter &simParams = project().m_simulationParameter;
	const NANDRAD::Location &location = project().m_location;

	// now set all values
	if (!simParams.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].empty())
		m_ui->lineEditInitialTemperature->setValue(simParams.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].get_value("C"));
	else
		m_ui->lineEditInitialTemperature->clear();

	if (!simParams.m_para[NANDRAD::SimulationParameter::P_InitialRelativeHumidity].empty())
		m_ui->lineEditInitialRelativeHumidity->setValue(simParams.m_para[NANDRAD::SimulationParameter::P_InitialRelativeHumidity].get_value("%"));
	else
		m_ui->lineEditInitialRelativeHumidity->clear();

	/// TODO: set remaining values

	// set percentages in spin boxes -> round to whole numbers!
	if (!simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone].empty())
		m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setValue((int)
					(simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone].value * 100));

	if (!simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionFloor].empty())
		m_ui->spinBoxSolarRadiationToFloor->setValue((int)
					(simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionFloor].value * 100));

	if (!simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionCeiling].empty())
		m_ui->spinBoxSolarRadiationToRoofCeiling->setValue((int)
					(simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionCeiling].value * 100));

	if (!simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionWalls].empty())
		m_ui->spinBoxSolarRadiationToWalls->setValue((int)
					(simParams.m_solarLoadsDistributionModel.m_para[NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionWalls].value * 100));

	m_ui->checkBoxUsePerez->blockSignals(true);
	m_ui->checkBoxUsePerez->setChecked(location.m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].isEnabled());
	m_ui->checkBoxUsePerez->blockSignals(false);

	m_ui->checkBoxEnableMoistureBalance->blockSignals(true);
	bool moistureEnabled = simParams.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled();
	m_ui->checkBoxEnableMoistureBalance->setChecked(moistureEnabled);
	m_ui->checkBoxEnableMoistureBalance->blockSignals(false);

	m_ui->lineEditInitialRelativeHumidity->setEnabled(moistureEnabled);
	m_ui->labelInitialRelHum->setEnabled(moistureEnabled);
	m_ui->labelInitialRelHumPerc->setEnabled(moistureEnabled);

	m_ui->comboBoxSolarDistributionModeltype->blockSignals(true);
	int idx = simParams.m_solarLoadsDistributionModel.m_distributionType;
	m_ui->comboBoxSolarDistributionModeltype->setCurrentIndex(idx);
	m_ui->comboBoxSolarDistributionModeltype->blockSignals(false);

	m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode->setEnabled(idx == 0);
	m_ui->spinBoxSolarRadiationToFloor->setEnabled(idx == 1);
	m_ui->spinBoxSolarRadiationToRoofCeiling->setEnabled(idx == 1);
	m_ui->spinBoxSolarRadiationToWalls->setEnabled(idx == 1);

	bool validFractions = ( m_ui->spinBoxSolarRadiationToFloor->value() +
						   m_ui->spinBoxSolarRadiationToRoofCeiling->value() +
						  m_ui->spinBoxSolarRadiationToWalls->value() ) == 100;
	m_ui->iconErrorFractionSum->setVisible(!validFractions && idx == 1);
	m_ui->labelErrorFractionSum->setVisible(!validFractions && idx == 1);
}


void SVSimulationModelOptions::onModified(int modificationType, ModificationInfo *) {
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::SolverParametersModified:
			updateUi();
		break;
		default:;
	}
}


void SVSimulationModelOptions::on_comboBoxSolarDistributionModeltype_currentIndexChanged(int index) {

	NANDRAD::SimulationParameter simParams = project().m_simulationParameter;
	simParams.m_solarLoadsDistributionModel.m_distributionType = (NANDRAD::SolarLoadsDistributionModel::distribution_t)index;

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("solar loads modified", project().m_solverParameter, simParams);
	undo->push();
}


void SVSimulationModelOptions::modifySolarLoadParameters(int paraEnum, const QSpinBox *spnBox) {

	NANDRAD::SimulationParameter simParams = project().m_simulationParameter;
	NANDRAD::KeywordList::setParameter(simParams.m_solarLoadsDistributionModel.m_para, "SolarLoadsDistributionModel::para_t", paraEnum, spnBox->value());

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("solar loads modified", project().m_solverParameter, simParams);
	undo->push();
}


void SVSimulationModelOptions::on_lineEditInitialTemperature_editingFinished() {

	if(!m_ui->lineEditInitialTemperature->isValid())
		return;

	NANDRAD::SimulationParameter simParams = project().m_simulationParameter;
	double val = m_ui->lineEditInitialTemperature->value();
	simParams.m_para[NANDRAD::SimulationParameter::P_InitialTemperature] = IBK::Parameter("InitialTemperature", val, IBK::Unit("C"));

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("solar loads modified", project().m_solverParameter, simParams);
	undo->push();
}


void SVSimulationModelOptions::on_spinBoxSolarRadiationToFloor_editingFinished() {
	modifySolarLoadParameters(NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionFloor, m_ui->spinBoxSolarRadiationToFloor);
}


void SVSimulationModelOptions::on_spinBoxSolarRadiationToRoofCeiling_editingFinished() {
	modifySolarLoadParameters(NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionCeiling, m_ui->spinBoxSolarRadiationToRoofCeiling);
}


void SVSimulationModelOptions::on_spinBoxSolarRadiationToWalls_editingFinished() {
	modifySolarLoadParameters(NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionWalls, m_ui->spinBoxSolarRadiationToWalls);
}


void SVSimulationModelOptions::on_spinBoxSolarRadiationGainsDirectlyToRoomNode_editingFinished() {
	modifySolarLoadParameters(NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone, m_ui->spinBoxSolarRadiationGainsDirectlyToRoomNode);
}


void SVSimulationModelOptions::on_checkBoxUsePerez_toggled(bool checked) {
	NANDRAD::Location location = project().m_location;

	if (checked)
		location.m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].set( NANDRAD::KeywordList::Keyword("Location::flag_t", NANDRAD::Location::F_PerezDiffuseRadiationModel), true);
	else
		location.m_flags[NANDRAD::Location::F_PerezDiffuseRadiationModel].clear();

	SVUndoModifyClimate *undo = new SVUndoModifyClimate("Radiation model modified", location, false);
	undo->push();
}


void SVSimulationModelOptions::on_checkBoxEnableMoistureBalance_toggled(bool checked) {

	NANDRAD::SimulationParameter simParams = project().m_simulationParameter;

	if (checked)
		simParams.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].set(
					NANDRAD::KeywordList::Keyword("SimulationParameter::flag_t", NANDRAD::SimulationParameter::F_EnableMoistureBalance), true);
	else
		simParams.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].clear();

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("Moisture balance modified", project().m_solverParameter, simParams);
	undo->push();
}

void SVSimulationModelOptions::onStyleChanged() {
	m_ui->labelErrorFractionSum->setStyleSheet("QLabel { color: #CC0000;  }");
}


void SVSimulationModelOptions::on_pushButtonPrecalculateViewFactors_clicked() {
	// TODO Stephan: select all surfaces that need view factors, or we calculate all surfaces?
	SVView3DDialog v3d;
	v3d.exportView3d();
}

