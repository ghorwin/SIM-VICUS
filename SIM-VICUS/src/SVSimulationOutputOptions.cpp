#include "SVSimulationOutputOptions.h"
#include "ui_SVSimulationOutputOptions.h"

SVSimulationOutputOptions::SVSimulationOutputOptions(QWidget *parent, NANDRAD::Outputs & outputs) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationOutputOptions),
	m_outputs(&outputs)
{
	m_ui->setupUi(this);
}


SVSimulationOutputOptions::~SVSimulationOutputOptions() {
	delete m_ui;
}


void SVSimulationOutputOptions::updateUi() {

}
