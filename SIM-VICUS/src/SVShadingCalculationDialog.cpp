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

#include "SVShadingCalculationDialog.h"
#include "ui_SVShadingCalculationDialog.h"

#include <QDate>
#include <QElapsedTimer>
#include <QTimer>

#include <QtExt_DateTimeInputDialog.h>

#include <vector>
#include <fstream>

#include <IBK_physics.h>
#include <IBK_Time.h>

#include <VICUS_Surface.h>
#include <VICUS_Project.h>


#include <SVProjectHandler.h>
#include <SVSimulationStartNandrad.h>
#include <SVSettings.h>
#include <SVMainWindow.h>

class ShadingCalculationProgress : public SH::Notification {
public:
	void notify() override {}
	void notify(double percentage) override;

	char				pad[7]; // fix padding, silences compiler warning
	QProgressDialog		*m_dlg = nullptr;
};

void ShadingCalculationProgress::notify(double percentage) {
	m_dlg->setValue((int)(m_dlg->maximum() * percentage));
	qApp->processEvents();
	if (m_dlg->wasCanceled())
		m_aborted = true;
}


SVShadingCalculationDialog::SVShadingCalculationDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVShadingCalculationDialog)
{
	m_ui->setupUi(this);

	// we set the duration time of the simulation
	m_ui->lineEditDuration->setup(m_ui->comboBoxUnitDuration, IBK::Unit("h"),
								  0, std::numeric_limits<double>::max(), tr("Duration of the simulation.") );

	m_ui->lineEditSunCone->setup(0, 45, tr("Half-angle of sun cone must be > 0 Deg!"), false, true);
	m_ui->lineEditGridSize->setup(0, 1000, tr("Grid size must be > 0 m!"), false, true);

	m_ui->comboBoxFileType->addItem( "tsv" );
	m_ui->comboBoxFileType->addItem( "d6o" );
	m_ui->comboBoxFileType->addItem( "d6b" );

	m_ui->radioButtonFast->toggle();

	// TODO : restore previously used setting
	m_ui->comboBoxFileType->setCurrentIndex( 0 );
}


SVShadingCalculationDialog::~SVShadingCalculationDialog() {
	delete m_ui;
}


int SVShadingCalculationDialog::edit() {

	// create a copy of the current project
	m_localProject = project();			// Mind: addresses to member variables m_solverParameters, m_location etc. do not change here!
	m_localProject.updatePointers();

	// initialize defaults
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvement

	// initialize simulation parameters with meaningful defaults and fix possibly wrong values
	// in project (wherever they come from)
	if (simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].name.empty() ||
		simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value < 0)
	{
		simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", 2019);
	}

	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].name.empty() ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value < 0 ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value > 365*24*3600)
	{
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].set("Start", 0, IBK::Unit("d"));
	}

	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty() ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_End].value < 0)
	{
		simParas.m_interval.m_para[ NANDRAD::Interval::P_End].set("End", 1, IBK::Unit("a"));
	}


	updateTimeFrameEdits();

	return exec();
	// if dialog was confirmed, data is transfered into project
}


void SVShadingCalculationDialog::updateTimeFrameEdits() {

	m_ui->lineEditStartDate->blockSignals(true);
	m_ui->lineEditEndDate->blockSignals(true);
	m_ui->lineEditDuration->blockSignals(true);

	// Note: we can be sure that all the parameters are set, though possibly to invalid values

	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	// fall-back to zero, if not specified
	double startOffset = simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value;

	IBK::Time t(startYear, startOffset);
	m_ui->lineEditStartDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	double endTime = simParas.m_interval.m_para[ NANDRAD::Interval::P_End].value;
	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty())
		endTime = startOffset + 365*24*3600; // fallback to 1 year
	double simDuration = endTime - startOffset;
	t += simDuration;

	m_ui->lineEditEndDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	IBK::Parameter durationPara;
	// use unit from end
	durationPara = IBK::Parameter("Duration", 0, simParas.m_interval.m_para[ NANDRAD::Interval::P_End].IO_unit);
	durationPara.value = simDuration; // set value in seconds
	m_ui->lineEditDuration->setFromParameter(durationPara);

	m_ui->lineEditStartDate->blockSignals(false);
	m_ui->lineEditEndDate->blockSignals(false);
	m_ui->lineEditDuration->blockSignals(false);
}


void SVShadingCalculationDialog::setSimulationParameters(const DetailType & dt) {

	m_ui->lineEditSunCone->setEnabled(false);
	m_ui->lineEditGridSize->setEnabled(false);

	double	gridSize = 0.1;	// size of grid in [m]
	double	sunCone = 3;	// half opening angle of the cone for sun mapping [Deg]

	switch (dt) {
		case Fast: {
			gridSize = 0.2;
			sunCone = 2;
		}
		break;
		case Detailed: {
			gridSize = 0.1;
			sunCone = 1;
		}
		break;
		case Manual: {
			m_ui->lineEditSunCone->setEnabled(true);
			m_ui->lineEditGridSize->setEnabled(true);
		}
		break;
	}

	m_ui->lineEditGridSize->setValue( gridSize );
	m_ui->lineEditSunCone->setValue( sunCone );
}


void SVShadingCalculationDialog::on_lineEditStartDate_editingFinished() {
	IBK::Time startTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditStartDate->text().toStdString());

	// update date time
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", startTime.year());
	simParas.m_interval.m_para[NANDRAD::Interval::P_Start].set("Start", startTime.secondsOfYear(), IBK::Unit("s"));
	updateTimeFrameEdits();
}


void SVShadingCalculationDialog::on_lineEditEndDate_editingFinished() {
	IBK::Time endTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditEndDate->text().toStdString());

	// compose start time (startYear and offset are given and well defined, we ensure that)
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = simParas.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// compute difference between dates
	IBK::Time diff = endTime - startTime; // Might be negative!
	if (!diff.isValid()) {
		m_ui->lineEditDuration->setValue(0); // set zero duration to indicate that something is wrong!
		return;
	}

	// end date is the offset from start, so we first need the start date
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].set("End", diff.secondsOfYear(), IBK::Unit("s"));
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = m_ui->lineEditDuration->currentUnit();

	updateTimeFrameEdits();
}


void SVShadingCalculationDialog::on_lineEditDuration_editingFinishedSuccessfully() {
	// we always update the end time and let the end time signal do the undo action stuff
	IBK::Parameter durPara = m_ui->lineEditDuration->toParameter("Duration");
	if (durPara.name.empty())
		return; // invalid input in parameter edit

	if (durPara.value <= 0)
		return; // invalid input in parameter edit

	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = simParas.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// add duration
	startTime += durPara.value;
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].set("End", startTime.secondsOfYear(), IBK::Unit("s"));
	// set duration unit in parameter - this will be used to select matching unit in combo box
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = durPara.IO_unit;
	updateTimeFrameEdits();
}


void SVShadingCalculationDialog::on_pushButtonCalculate_clicked(){
	try {
		calculateShadingFactors();
	} catch (IBK::Exception &ex) {

		QMessageBox msgBox(QMessageBox::Critical, "Error", ex.what(), QMessageBox::Ok, this);
		msgBox.exec();

	}
}


void SVShadingCalculationDialog::calculateShadingFactors() {
	FUNCID(SVShadingCalculationDialog::calculateShadingFactors);

	std::vector<std::vector<IBKMK::Vector3D> > selObst;
	std::vector<std::vector<IBKMK::Vector3D> > selSurf;

	// We take all our selected surfaces
	project().selectedSurfaces(m_selSurfaces,VICUS::Project::SG_Building);
	project().selectedSurfaces(m_selObstacles,VICUS::Project::SG_Obstacle);

	const NANDRAD::Location &loc = project().m_location;

	const NANDRAD::SimulationParameter &simuPara = project().m_simulationParameter;

	IBK::IntPara startYear = simuPara.m_intPara[NANDRAD::SimulationParameter::IP_StartYear];
	IBK::Parameter startDay = simuPara.m_interval.m_para[NANDRAD::Interval::P_Start];
	IBK::Parameter endDay = simuPara.m_interval.m_para[NANDRAD::Interval::P_End];

	startYear = IBK::IntPara("Start Year", 2007);
	startDay = IBK::Parameter("Start Day", 0, IBK::Unit("d") );
	endDay = IBK::Parameter("End Day", 365, IBK::Unit("d") );

	if ( startYear.empty() )
		throw IBK::Exception( IBK::FormatString("Start year of simulation has not been set.") , FUNC_ID );

	if ( startDay.empty() )
		throw IBK::Exception( IBK::FormatString("Start day of simulation has not been set.") , FUNC_ID );

	if ( endDay.empty() )
		throw IBK::Exception( IBK::FormatString("End day of simulation has not been set.") , FUNC_ID );

	IBK::Time simTimeStart (startYear.value, startDay.value );
	IBK::Time simTimeEnd (startYear.value, endDay.value );

	unsigned int durationInSec = (unsigned int)simTimeStart.secondsUntil(simTimeEnd);
	double sunConeDeg = m_ui->lineEditSunCone->value();

	OutputType outputType = (OutputType)m_ui->comboBoxFileType->currentIndex();



//	m_shading.initializeShadingCalculation(loc.m_timeZone,
//										   loc.m_para[NANDRAD::Location::P_Longitude].value/IBK::DEG2RAD,
//										   loc.m_para[NANDRAD::Location::P_Latitude].value/IBK::DEG2RAD,
//										   simTimeStart,
//										   durationInSec,
//										   1800, // TODO : Stephan, get from UI input
//										   sunConeDeg );

	m_shading.initializeShadingCalculation(1,
										   14.27,
										   50,
										   simTimeStart,
										   durationInSec,
										   1800, // TODO : Stephan, get from UI input
										   sunConeDeg );


	// *** compose vectors with obstacles

	for (const VICUS::Surface *s: m_selObstacles)
		selObst.push_back( s->geometry().polygon().vertexes() );

	// *** compose vector with selected surfaces
	std::vector<unsigned int> surfaceIDs; // holds IDs of calculated surfaces
	for (const VICUS::Surface *s: m_selSurfaces) {

		if ( s->m_componentInstance == nullptr )
			continue;  // skip invalid surfaces - surfaces without component are not computed in calculation and thus do not require shading factors

		// we want to take only surface connected to ambient, that means, the associated component instance
		// must have one zone with ID 0 assigned
		if (s->m_componentInstance->m_sideASurfaceID != VICUS::INVALID_ID &&
				s->m_componentInstance->m_sideBSurfaceID != VICUS::INVALID_ID)
			continue; // skip inside constructions

		// we compute shading factors for this surface
		selSurf.push_back( s->geometry().polygon().vertexes() );
		surfaceIDs.push_back(s->m_id);

		// Mind: surface planes may also shade other surfaces
		selObst.push_back( s->geometry().polygon().vertexes() );
	}

	m_shading.setGeometry(selSurf, selObst);

	QProgressDialog progressDialog(tr("Calculate shading factors"), tr("Abort"), 0, 100, this);
	progressDialog.setValue(0);
	progressDialog.show();

//	QElapsedTimer progressTimer;
//	progressTimer.start();

	ShadingCalculationProgress progressNotifyer;
	progressNotifyer.m_dlg = &progressDialog;

	// *** compute shading ***

	double gridSize = m_ui->lineEditGridSize->value();
	m_shading.calculateShadingFactors(&progressNotifyer, gridSize);

	if (progressNotifyer.m_aborted)
		return;
	SVProjectHandler &prj = SVProjectHandler::instance();

	// TODO Stephan, what if project hasn't been saved yet? projectName is empty then

	QString projectName = QFileInfo(prj.projectFile()).completeBaseName();
	QString shadingPath = QFileInfo(prj.projectFile()).dir().filePath(projectName) + '/';

	QDir shadingDir (shadingPath);


	if ( !shadingDir.exists() && !shadingDir.mkdir(shadingPath) )
		throw IBK::Exception( IBK::FormatString("Could not create directory '%1'").arg(shadingPath.toStdString()), FUNC_ID );

	switch ( outputType ) {
		case TsvFile : {
			QString pathTSV = shadingPath  + projectName + "_shadingFactors.tsv" ;
			IBK::Path exportFileTSV(pathTSV.toStdString() );
			m_shading.writeShadingFactorsToTSV(exportFileTSV, surfaceIDs);
		} break;
		case D6oFile : {
			QString pathD6O = shadingPath  + projectName + "_shadingFactors.d6o" ;
			IBK::Path exportFileD6O(pathD6O.toStdString() );
			m_shading.writeShadingFactorsToDataIO(exportFileD6O, surfaceIDs, false);
		} break;
		case D6bFile : {
			QString pathD6B = shadingPath  + projectName + "_shadingFactors.d6b" ;
			IBK::Path exportFileD6B(pathD6B.toStdString() );
			m_shading.writeShadingFactorsToDataIO(exportFileD6B, surfaceIDs, true);
		} break;
	}

}

void SVShadingCalculationDialog::on_pushButtonChangeTime_clicked() {
	m_simStartWidget->exec();
}

void SVShadingCalculationDialog::on_radioButtonFast_toggled(bool checked) {
	setSimulationParameters(DetailType::Fast);
}

void SVShadingCalculationDialog::on_radioButtonManual_toggled(bool checked) {
	setSimulationParameters(DetailType::Manual);
}

void SVShadingCalculationDialog::on_radioButtonDetailed_toggled(bool checked) {
	setSimulationParameters(DetailType::Detailed);
}
