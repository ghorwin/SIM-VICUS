#include "SVSimulationModelOptions.h"
#include "ui_SVSimulationModelOptions.h"

#include <NANDRAD_KeywordList.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_SimulationParameter.h>

SVSimulationModelOptions::SVSimulationModelOptions(QWidget *parent, NANDRAD::SimulationParameter & simParams) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationModelOptions),
	m_simParams(&simParams)
{
	m_ui->setupUi(this);

	m_ui->mainLayout->setMargin(0);

}


SVSimulationModelOptions::~SVSimulationModelOptions() {
	delete m_ui;
}


void SVSimulationModelOptions::updateUi() {

	// generate defaults
	NANDRAD::SimulationParameter s;
	s.initDefaults();

}


