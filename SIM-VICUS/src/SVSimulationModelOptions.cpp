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

	m_ui->mainLayout->setMargin(0);

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

	m_ui->lineEditInitialTemperature->setValue(m_simParams->m_para[NANDRAD::SimulationParameter::P_InitialTemperature].get_value("C"));
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
