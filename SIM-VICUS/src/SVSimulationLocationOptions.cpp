#include "SVSimulationLocationOptions.h"
#include "ui_SVSimulationLocationOptions.h"

#include "SVSettings.h"

SVSimulationLocationOptions::SVSimulationLocationOptions(QWidget *parent, NANDRAD::Location & location) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationLocationOptions),
	m_location(&location)
{
	m_ui->setupUi(this);
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateUi() {
	// parse climate dabase paths, read in CCM files and populate the combo boxes
	SVSettings::instance().updateClimateFileList();

}
