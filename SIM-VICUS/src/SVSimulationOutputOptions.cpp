#include "SVSimulationOutputOptions.h"
#include "ui_SVSimulationOutputOptions.h"

#include <VICUS_Outputs.h>
#include <VICUS_KeywordList.h>

SVSimulationOutputOptions::SVSimulationOutputOptions(QWidget *parent, VICUS::Outputs & outputs) :
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
	m_ui->checkBoxDefaultZoneOutputs->setChecked(m_outputs->m_flags[VICUS::Outputs::OF_CreateDefaultZoneOutputs].isEnabled());
}


void SVSimulationOutputOptions::on_checkBoxDefaultZoneOutputs_toggled(bool checked) {
	if (checked)
		m_outputs->m_flags[VICUS::Outputs::OF_CreateDefaultZoneOutputs]
				.set(VICUS::KeywordList::Keyword("Outputs::flag_t", VICUS::Outputs::OF_CreateDefaultZoneOutputs), checked);
	else
		m_outputs->m_flags[VICUS::Outputs::OF_CreateDefaultZoneOutputs].clear();
}
