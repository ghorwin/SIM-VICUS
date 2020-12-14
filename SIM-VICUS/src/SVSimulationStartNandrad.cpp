#include "SVSimulationStartNandrad.h"
#include "ui_SVSimulationStartNandrad.h"

#include <QHBoxLayout>

#include <SVProjectHandler.h>
#include <VICUS_Project.h>

#include "SVSimulationPerformanceOptions.h"

SVSimulationStartNandrad::SVSimulationStartNandrad(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationStartNandrad)
{
	m_ui->setupUi(this);

	{
		m_simulationPerformanceOptions = new SVSimulationPerformanceOptions(this, m_solverParams);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationPerformanceOptions);
		m_ui->tabPerformanceOptions->setLayout(h);
	}
}


SVSimulationStartNandrad::~SVSimulationStartNandrad() {
	delete m_ui;
}


int SVSimulationStartNandrad::edit() {
	// store current project settings
//	m_solverParams = project().m_solverParameter;
	m_simulationPerformanceOptions->updateUi();

	return exec();
}
