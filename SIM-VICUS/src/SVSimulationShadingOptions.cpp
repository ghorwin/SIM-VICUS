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

#include "SVSimulationShadingOptions.h"
#include "ui_SVSimulationShadingOptions.h"

#include <QDate>
#include <QElapsedTimer>
#include <QTimer>

#include <QtExt_DateTimeInputDialog.h>

#include <vector>
#include <fstream>

#include <IBK_physics.h>
#include <IBK_Time.h>
#include <IBKMK_Polygon2D.h>

#include <VICUS_Surface.h>
#include <VICUS_Project.h>

#include <SVProjectHandler.h>
#include <SVSimulationStartNandrad.h>
#include <SVSettings.h>
#include <SVMainWindow.h>

#include <SH_StructuralShading.h>

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


SVSimulationShadingOptions::SVSimulationShadingOptions(QWidget *parent, NANDRAD::SimulationParameter & simParams, NANDRAD::Location & location) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationShadingOptions),
	m_simParams(&simParams),
	m_location(&location),
	m_shading(new SH::StructuralShading)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->lineEditSunCone->setup(0, 45, tr("Half-angle of sun cone must be > 0 Deg!"), false, true);
	m_ui->lineEditGridSize->setup(0, 1000, tr("Grid size must be > 0 m!"), false, true);
	m_ui->lineEditSteps->setup(1, 60, tr("Steps per hour must be between 1 and 60!"), true, true);

	m_ui->comboBoxFileType->addItem( "tsv" );
	m_ui->comboBoxFileType->addItem( "d6o" );
	m_ui->comboBoxFileType->addItem( "d6b" );

	m_ui->radioButtonFast->toggle();

	// TODO : restore previously used setting
	m_ui->comboBoxFileType->setCurrentIndex( 0 );
}


SVSimulationShadingOptions::~SVSimulationShadingOptions() {
	delete m_ui;
	delete m_shading;
}


void SVSimulationShadingOptions::updateUi() {
	updateFileName();
}


void SVSimulationShadingOptions::setSimulationParameters(const DetailType & dt) {

	m_ui->lineEditSunCone->setReadOnly(true);
	m_ui->lineEditGridSize->setReadOnly(true);

	double	gridSize = 0.05;	// size of grid in [m]
	double	sunCone = 0.01;	// half opening angle of the cone for sun mapping [Deg]
	unsigned int stepCount = 1;

	switch (dt) {
		case Fast: {
			gridSize = 0.1;
			sunCone = 2;
		}
		break;
		case Detailed: {
			gridSize = 0.05;
			sunCone = 0.5;
		}
		break;
		case Manual: {
			m_ui->lineEditSunCone->setReadOnly(false);
			m_ui->lineEditGridSize->setReadOnly(false);
		}
		break;
	}

	m_ui->lineEditGridSize->setValue( gridSize );
	m_ui->lineEditSunCone->setValue( sunCone );
	m_ui->lineEditSteps->setValue( stepCount );

	updateFileName();
}


void SVSimulationShadingOptions::on_pushButtonCalculate_clicked(){
	try {
		calculateShadingFactors();
	}
	catch (IBK::Exception &ex) {
		QMessageBox msgBox(QMessageBox::Critical, QString(), tr("The calculation was aborted or an error has occurred "
																"during the calculation of shading factors."
																), QMessageBox::Ok, this);
		msgBox.setDetailedText(ex.what());
		msgBox.exec();
	}
}


void SVSimulationShadingOptions::calculateShadingFactors() {
//	FUNCID(SVSimulationShadingOptions::calculateShadingFactors);

	std::vector<SH::StructuralShading::ShadingObject> selObst;
	std::vector<SH::StructuralShading::ShadingObject> selSurf;

	// We take all our selected surfaces
//	if (m_useOnlySelectedSurfaces) {
	if (/* DISABLES CODE */ (true)) {
		project().selectedSurfaces(m_selSurfaces,VICUS::Project::SG_Building);
		project().selectedSubSurfaces(m_selSubSurfaces,VICUS::Project::SG_Building);
		project().selectedSurfaces(m_selObstacles,VICUS::Project::SG_Obstacle);
	}
	else {
		std::set<const VICUS::Object*> sel;
		// take all, regardless of visibility or selection state
		project().selectObjects(sel, VICUS::Project::SelectionGroups(VICUS::Project::SG_Building | VICUS::Project::SG_Obstacle), false, false);
		// filter out surfaces
		m_selSurfaces.clear();
		m_selObstacles.clear();
		for (const VICUS::Object* o : sel) {
			const VICUS::Surface * surf = dynamic_cast<const VICUS::Surface*>(o);
			if (surf != nullptr) {
				if (surf->m_parent != nullptr) {
					m_selSurfaces.push_back(surf);

					if(!surf->subSurfaces().empty()) {
						for (const VICUS::SubSurface &ss : surf->subSurfaces()){
							m_selSubSurfaces.push_back(&ss);
						}
					}
				}
				else
					m_selObstacles.push_back(surf);
			}
		}
	}

	const NANDRAD::Location &loc = *m_location;
	const NANDRAD::SimulationParameter &simuPara = *m_simParams;

	try {
		simuPara.m_interval.checkParameters();
	} catch (...) {
		QMessageBox::critical(this, QString(), tr("Simulation time interval is not properly configured. Please set a valid simulation time interval and "
												  "compute shading afterwards!"));
		return;
	}

	if ( !m_ui->lineEditGridSize->isValid() ) {
		QMessageBox::critical(this, QString(), "Grid size must be > 0 m!");
		return;
	}

	if ( !m_ui->lineEditSunCone->isValid() ) {
		QMessageBox::critical(this, QString(), "Half-angle of sun cone must be > 0 Deg!");
		return;
	}

	if ( !m_ui->lineEditSteps->isValid() ) {
		QMessageBox::critical(this, QString(), "Steps per hour must be between 1 and 60!");
		return;
	}

	IBK::IntPara startYear = simuPara.m_intPara[NANDRAD::SimulationParameter::IP_StartYear];
	IBK::Parameter startDay = simuPara.m_interval.m_para[NANDRAD::Interval::P_Start];
	IBK::Parameter endDay = simuPara.m_interval.m_para[NANDRAD::Interval::P_End];

	IBK::Time simTimeStart (startYear.value, startDay.value );
	IBK::Time simTimeEnd (startYear.value, endDay.value );

	unsigned int durationInSec = (unsigned int)simTimeStart.secondsUntil(simTimeEnd);
	double sunConeDeg = m_ui->lineEditSunCone->value();

	double stepDuration = 3600/m_ui->lineEditSteps->value();


	m_shading->initializeShadingCalculation(loc.m_timeZone,
										   loc.m_para[NANDRAD::Location::P_Longitude].get_value("Deg"),
										   loc.m_para[NANDRAD::Location::P_Latitude].get_value("Deg"),
										   simTimeStart,
										   durationInSec,
										   stepDuration,
										   sunConeDeg );


	// *** compose vectors with obstacles

	for (const VICUS::Surface *s: m_selObstacles)
		selObst.push_back( SH::StructuralShading::ShadingObject(s->uniqueID(),
																IBKMK::Polygon3D(s->geometry().polygon().vertexes() ),
																true) );

	// *** compose vector with selected surfaces
	std::vector<unsigned int> surfaceIDs; // holds IDs of calculated surfaces
	for (const VICUS::Surface *s: m_selSurfaces) {

		if ( s->m_componentInstance == nullptr )
			continue;  // skip invalid surfaces - surfaces without component are not computed in calculation and thus do not require shading factors

		// we want to take only surface connected to ambient, that means, the associated component instance
		// must have one zone with ID 0 assigned
		if (s->m_componentInstance->m_idSideASurface != VICUS::INVALID_ID &&
			s->m_componentInstance->m_idSideBSurface != VICUS::INVALID_ID)
			continue; // skip inside constructions

		// we compute shading factors for this surface
		selSurf.push_back( SH::StructuralShading::ShadingObject(s->uniqueID(),
																IBKMK::Polygon3D(s->geometry().polygon().vertexes() ),
																s->m_parent == nullptr) );
		surfaceIDs.push_back(s->m_id);

		// Mind: surface planes may also shade other surfaces
		selObst.push_back( SH::StructuralShading::ShadingObject(s->uniqueID(),
																IBKMK::Polygon3D(s->geometry().polygon().vertexes() ),
																s->m_parent == nullptr) );
	}
	for (const VICUS::SubSurface *ss: m_selSubSurfaces) {

		if ( ss->m_subSurfaceComponentInstance == nullptr )
			continue;  // skip invalid surfaces - surfaces without component are not computed in calculation and thus do not require shading factors

		if ( ss->m_parent == nullptr ) // no parent; should not be possible but checked anyway
			continue;

		const IBKMK::Vector3D &offset3D = dynamic_cast<const VICUS::Surface*>(ss->m_parent)->geometry().offset();

		const IBKMK::Vector3D &localX = dynamic_cast<const VICUS::Surface*>(ss->m_parent)->geometry().localX();
		const IBKMK::Vector3D &localY = dynamic_cast<const VICUS::Surface*>(ss->m_parent)->geometry().localY();


		qDebug() << "======================================================";
		qDebug() << QString::fromStdString(ss->m_displayName.toStdString());

		// we need to calculate the 3D Points of the Sub Surface
		std::vector<IBKMK::Vector3D> subSurf3D;
		for (unsigned int i=0; i<ss->m_polygon2D.vertexes().size(); ++i) {
			const IBKMK::Vector2D &vertex = ss->m_polygon2D.vertexes()[i];
			subSurf3D.push_back(offset3D + localX*vertex.m_x + localY*vertex.m_y);
			qDebug() << i << "\t" << subSurf3D[i].m_x << "\t" << subSurf3D[i].m_y << "\t" << subSurf3D[i].m_z;
		}

		// we compute shading factors for this surface
		surfaceIDs.push_back(ss->m_id);
		selSurf.push_back(SH::StructuralShading::ShadingObject(ss->uniqueID(),
															   IBKMK::Polygon3D(subSurf3D),
															   false) );
	}

	m_shading->setGeometry(selSurf, selObst);

	QProgressDialog progressDialog(tr("Calculate shading factors"), tr("Abort"), 0, 100, this);
	progressDialog.setValue(0);
	progressDialog.show();

//	QElapsedTimer progressTimer;
//	progressTimer.start();

	ShadingCalculationProgress progressNotifyer;
	progressNotifyer.m_dlg = &progressDialog;

	// *** compute shading ***

	double gridSize = m_ui->lineEditGridSize->value();
	m_shading->calculateShadingFactors(&progressNotifyer, gridSize);

	if (progressNotifyer.m_aborted) {
		return;
	}

	SVProjectHandler &prj = SVProjectHandler::instance();
	QDir projectDir = QFileInfo(prj.projectFile()).dir();

	OutputType outputType = (OutputType)m_ui->comboBoxFileType->currentIndex();
	switch ( outputType ) {
		case TsvFile : {
			IBK::Path exportFileTSV( (projectDir.absoluteFilePath(m_shadingFactorBaseName) + ".tsv").toStdString() );
			m_shading->writeShadingFactorsToTSV(exportFileTSV, surfaceIDs);
		} break;
		case D6oFile : {
			QString pathD6O = projectDir.absoluteFilePath(m_shadingFactorBaseName) + ".d6o" ;
			IBK::Path exportFileD6O(pathD6O.toStdString() );
			m_shading->writeShadingFactorsToDataIO(exportFileD6O, surfaceIDs, false);
		} break;
		case D6bFile : {
			QString pathD6B = projectDir.absoluteFilePath(m_shadingFactorBaseName) + ".d6b" ;
			IBK::Path exportFileD6B(pathD6B.toStdString() );
			m_shading->writeShadingFactorsToDataIO(exportFileD6B, surfaceIDs, true);
		} break;
	}
}


void SVSimulationShadingOptions::on_radioButtonFast_toggled(bool checked) {
	if (checked)
		setSimulationParameters(DetailType::Fast);
}


void SVSimulationShadingOptions::on_radioButtonManual_toggled(bool checked) {
	if (checked)
		setSimulationParameters(DetailType::Manual);
}


void SVSimulationShadingOptions::on_radioButtonDetailed_toggled(bool checked) {
	if (checked)
		setSimulationParameters(DetailType::Detailed);
}


void SVSimulationShadingOptions::updateFileName() {
	SVProjectHandler &prj = SVProjectHandler::instance();

	QString projectName = QFileInfo(prj.projectFile()).completeBaseName();

//	QString hashCode = "0815";

//	m_shadingFactorBaseName = projectName + "_shadingFactors_" + hashCode;
	m_shadingFactorBaseName = projectName + "_shadingFactors";

	OutputType outputType = (OutputType)m_ui->comboBoxFileType->currentIndex();
	QString shadingFilePath = m_shadingFactorBaseName;
	switch ( outputType ) {
		case TsvFile : shadingFilePath += ".tsv"; break;
		case D6oFile : shadingFilePath += ".d6o"; break;
		case D6bFile : shadingFilePath += ".d6b"; break;
	}

	m_ui->lineEditShadingFactorFilename->setText(shadingFilePath);
}


void SVSimulationShadingOptions::on_comboBoxFileType_currentIndexChanged(int /*index*/) {
	updateFileName();
}

