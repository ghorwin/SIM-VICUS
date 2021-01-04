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
