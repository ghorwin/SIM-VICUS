#include "SVSimulationStartNandrad.h"
#include "ui_SVSimulationStartNandrad.h"

SVSimulationStartNandrad::SVSimulationStartNandrad(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SVSimulationStartNandrad)
{
	ui->setupUi(this);
}

SVSimulationStartNandrad::~SVSimulationStartNandrad()
{
	delete ui;
}
