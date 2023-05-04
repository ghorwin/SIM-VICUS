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
#include <QButtonGroup>
#include <QFile>

#include <QtExt_DateTimeInputDialog.h>

#include <vector>
#include <fstream>

#include <IBK_physics.h>
#include <IBK_Time.h>

#include <IBKMK_Polygon2D.h>

#include <VICUS_Surface.h>
#include <VICUS_Project.h>
#include <VICUS_utilities.h>
#include "VICUS_Constants.h"

#include <SH_StructuralShading.h>

#include "SVProjectHandler.h"
#include "SVSimulationStartNandrad.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVStyle.h"
#include "SVClimateDataTableModel.h"
#include "SVSettings.h"

std::vector<IBKMK::Polygon2D> convertVicus2IBKMKPolyVector(const std::vector<VICUS::PlaneGeometry::Hole> holePolys) {
	std::vector<IBKMK::Polygon2D> ibkmkHolePolys;
    for(const VICUS::PlaneGeometry::Hole &h : holePolys) {
        const VICUS::Polygon2D &poly2D = h.m_holeGeometry;
		poly2D.vertexes();
		ibkmkHolePolys.push_back(poly2D);
	}
	return ibkmkHolePolys;
}

void debugPolygonPoints(QString preText, const IBKMK::Polygon3D &poly) {
	for(const IBKMK::Vector3D &v3D : poly.vertexes())
		qDebug() << preText << "\tx: " << (double)((int)(1000.0*v3D.m_x))/1000.0
							<< "\ty: " << (double)((int)(1000.0*v3D.m_y))/1000.0
							<< "\tz: " << (double)((int)(1000.0*v3D.m_z))/1000.0;
}

class ShadingCalculationProgress : public SH::Notification {
	Q_DECLARE_TR_FUNCTIONS(ShadingCalculationProgress)
public:

	void notify() override {}
	void notify(double percentage) override;

	char				pad[7]; // fix padding, silences compiler warning
	QProgressDialog		*m_dlg = nullptr;
	QTime				m_startTime;
	QString				m_labelText;
};

void ShadingCalculationProgress::notify(double percentage) {
	m_dlg->setValue((int)(m_dlg->maximum() * percentage));
	QTime currTime = QTime::currentTime();
	int usedSecs = m_startTime.secsTo(currTime);
	int remainingSecs;
	if (percentage == 0.0)
		remainingSecs = 999;
	else
		remainingSecs = (int)((double)usedSecs * (1 - percentage) / percentage);

	QTime remainingTime (0,0,0);
	remainingTime = remainingTime.addSecs(remainingSecs);
	QString labelText = tr("Calculating Shading factors\nRemaining time: %1")
							.arg(remainingTime.toString() );
	m_dlg->setLabelText(labelText);
	qApp->processEvents();
	if (m_dlg->wasCanceled())
		m_aborted = true;
}


SVSimulationShadingOptions::SVSimulationShadingOptions(QWidget *parent, NANDRAD::SimulationParameter & simParams, NANDRAD::Location & location) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationShadingOptions),
	m_simParams(&simParams),
	m_location(&location),
	m_shading(new SH::StructuralShading)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutWidget->setMargin(0);

	QPalette p;

	p.setColor(QPalette::Text, SVStyle::instance().m_logErrorText);
	m_ui->labelInputError->setPalette(p);

	m_ui->lineEditSunCone->setup(0, 45, tr("Half-angle of sun cone must be > 0 Deg!"), false, true);
	m_ui->lineEditGridSize->setup(0, 1000, tr("Grid size must be > 0 m!"), false, true);
	m_ui->lineEditSteps->setup(1, 60, tr("Steps per hour must be between 1 and 60!"), true, true);

	m_ui->comboBoxFileType->addItem( "tsv" );
	m_ui->comboBoxFileType->addItem( "d6o" );
	m_ui->comboBoxFileType->addItem( "d6b" );

	m_ui->radioButtonFast->toggle();

	// TODO : restore previously used setting
	m_ui->comboBoxFileType->setCurrentIndex( 0 );

	QButtonGroup *bg = new QButtonGroup;
	bg->addButton(m_ui->radioButtonFlatGeometry);
	bg->addButton(m_ui->radioButtonExtrudedGeometry);

	QButtonGroup *bgMethod = new QButtonGroup;
	bgMethod->addButton(m_ui->radioButtonRayTracing);
	bgMethod->addButton(m_ui->radioButtonSurfaceClipping);

	m_ui->radioButtonFlatGeometry->toggle();
	m_ui->radioButtonRayTracing->toggle();
}


SVSimulationShadingOptions::~SVSimulationShadingOptions() {
	delete m_ui;
	delete m_shading;
}


void SVSimulationShadingOptions::updateUi() {


	// TODO Stephan: Zustand des Dialogs Optionen für Verschattungsberechnung im VICUS Projekt speichern
	// Comboboxes statt RadioButtons, kurze Beschreibung der Optionen "Ray-Tracing (macht xy)" und "SurfaceClipping (macht das so besser)"

	updateShadingFileName();

	// Here we check all parameters that are *not* edited in this dialog, and show
	// a summary of these input parameters.
	// If any of the input data is invalid, we disable the "calculate" button and show
	// an error message.

	// we are pessimistic at first
	m_ui->labelInputError->setVisible(true);
	m_ui->pushButtonCalculate->setEnabled(false);
	m_ui->labelLatitude->setText("---");
	m_ui->labelLongitude->setText("---");
	m_ui->labelStartTime->setText("---");
	m_ui->labelEndTime->setText("---");

	QString errorColor = SVStyle::instance().m_logErrorText.name();

	// *** simulation time interval ***

	const NANDRAD::SimulationParameter &simuPara = *m_simParams;
	try {
		simuPara.m_interval.checkParameters();
	} catch (...) {
		m_ui->labelInputError->setText( tr("<span style=\"color:%1\">Simulation time interval is not properly configured. Please set a valid simulation time interval and "
										   "compute shading afterwards!</span>").arg(errorColor) );
		return;
	}
	IBK::IntPara startYear = simuPara.m_intPara[NANDRAD::SimulationParameter::IP_StartYear];
	IBK::Parameter startDay = simuPara.m_interval.m_para[NANDRAD::Interval::P_Start];
	IBK::Parameter endDay = simuPara.m_interval.m_para[NANDRAD::Interval::P_End];
	// startDay and endDay are valid, since they have been checked and fixed already in SVSimulationStartNandrad.cpp

	m_startTime =  IBK::Time(startYear.value, startDay.value );
	m_endTime = IBK::Time(startYear.value, endDay.value );
	m_ui->labelStartTime->setText(QString::fromStdString(startDay.toString(IBK::Unit("d"))));
	m_ui->labelEndTime->setText(QString::fromStdString(endDay.toString(IBK::Unit("d"))));

	m_durationInSec = (unsigned int)m_startTime.secondsUntil(m_endTime);

	// *** location ***

	const NANDRAD::Location &loc = *m_location;
	try {
		// check for valid climate data file path and optionally given (alternative) latitute and longitude
		loc.checkParameters();
	} catch (...) {
		m_ui->labelInputError->setText( tr("<span style=\"color:%1\">Location data is not correctly defined. Please open the location data dialog and specify "
										   "a valid climate data file and/or building location.</span>").arg(errorColor) );
		return;
	}
	// we need longitude and latitude
	m_longitudeInDeg = loc.m_para[NANDRAD::Location::P_Longitude].get_value("Deg");
	m_latitudeInDeg = loc.m_para[NANDRAD::Location::P_Latitude].get_value("Deg");

	m_ui->labelLatitude->setText( QString("%L1").arg(m_latitudeInDeg, 0, 'f', 2));
	m_ui->labelLongitude->setText( QString("%L1").arg(m_longitudeInDeg, 0, 'f', 2));

	// all checks ok, hide the error text and enable the button
	m_ui->labelInputError->setVisible(false);
	m_ui->pushButtonCalculate->setEnabled(true);

	// update the shading file infos
	setPreviousSimulationFileValues();
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

	updateShadingFileName();
}


void SVSimulationShadingOptions::setPreviousSimulationFileValues() {

	SVProjectHandler &prj = SVProjectHandler::instance();
	QDir projectDir = QFileInfo(prj.projectFile()).dir();
	std::string exportFileBaseName = projectDir.absoluteFilePath(m_shadingFactorBaseName).toStdString();

	const std::vector<std::string> fileEndings = { ".tsv", ".d6o", ".d6b"};

	for (unsigned int i = 0; i < fileEndings.size(); ++i) {
		// check if a previous file with one of the endings in fileEndings exists
		QFileInfo currentFile ( QString::fromStdString(exportFileBaseName + fileEndings[i]) );
		if (currentFile.exists()){
			// the file exists, read the meta data and display in the corresponding labels
			m_ui->labelPreviousShadingFile->setText(m_shadingFactorBaseName + fileEndings[i].c_str());
			QDateTime creationTime = currentFile.lastModified();
			m_ui->labelPreviousShadingFileCreationDate->setText(creationTime.toString("dd.MM.yyyy hh:mm"));
			m_ui->pushButtonDeletePreviousShadingFile->setEnabled(true);
			return;
		}
	}

	// no previous file exists
	m_ui->labelPreviousShadingFile->setText(tr("No shading file, yet!"));
	m_ui->labelPreviousShadingFileCreationDate->setText("---");
	m_ui->pushButtonDeletePreviousShadingFile->setEnabled(false);
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

	bool useClipping = m_ui->radioButtonSurfaceClipping->isChecked();

	if ( !m_ui->lineEditGridSize->isValid() ) {
		QMessageBox::critical(this, QString(), tr("Grid size must be > 0 m!"));
		return;
	}

	if ( !m_ui->lineEditSunCone->isValid() ) {
		QMessageBox::critical(this, QString(), tr("Half-angle of sun cone must be > 0 Deg!"));
		return;
	}

	if ( !m_ui->lineEditSteps->isValid() ) {
		QMessageBox::critical(this, QString(), tr("Steps per hour must be between 1 and 60!"));
		return;
	}

	// We take all our selected surfaces
	if (m_ui->radioButtonSelectedGeometry->isChecked()) {
		project().selectedSurfaces(m_selSurfaces,VICUS::Project::SG_Building);
		project().selectedSubSurfaces(m_selSubSurfaces,VICUS::Project::SG_Building);
		project().selectedSurfaces(m_selObstacles,VICUS::Project::SG_Obstacle);

		// we need to handle the case that we have a climate data file path, but no longitude/latitude given
		if ( m_selSurfaces.empty() && m_selSubSurfaces.empty() ) {
			QMessageBox::critical(this, QString(), tr("No (sub-)surfaces have been selected."
													  "Note that shading factors won't be generated for obstacles/annonymous geometry!"));
			return;
		}
	}
	else {
		std::set<const VICUS::Object*> sel;
		// take all, regardless of visibility or selection state
		project().selectObjects(sel, VICUS::Project::SelectionGroups(VICUS::Project::SG_Building | VICUS::Project::SG_Obstacle), false, false);
		// filter out surfaces
		m_selSurfaces.clear();
		m_selSubSurfaces.clear();
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
		if (m_selSurfaces.empty() && m_selSubSurfaces.empty()) {
			QMessageBox::critical(this, QString(), tr("The current project does not contain any surfaces to compute shading factors for. "
													  "Note that shading factors won't be generated for obstacles/annonymous geometry!"));
			return;
		}
		if (SVSettings::instance().showDoNotShowAgainQuestion(this, "shading-calculation-with-entire-geometry", QString(),
															  tr("Shading calculation will be done using all surfaces of the project, regardless of whether "
																 "they are currently visible or not. Currently %1 surfaces, %2 sub-surfaces and %3 obstacles have been selected. "
																 "For large projects this may lead to large simulation time. Continue?")
															  .arg(m_selSurfaces.size()).arg(m_selSubSurfaces.size()).arg(m_selObstacles.size()),
															  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;
	}

	const NANDRAD::Location &loc = *m_location;
	// all checks have been made already in updateUi()
	// if we have no location yet, read climate data file and extract location
	if (loc.m_para[NANDRAD::Location::P_Longitude].name.empty()) {
		// Mind: loc.m_climateFilePath contains placeholders in case of database files.
		const SVClimateFileInfo * dbFileInfo = SVSettings::instance().climateDataTableModel()->infoForFilename(QString::fromStdString(loc.m_climateFilePath.str()));
		if (dbFileInfo != nullptr) {
			m_latitudeInDeg = dbFileInfo->m_latitudeInDegree;
			m_longitudeInDeg = dbFileInfo->m_longitudeInDegree;
		}
		else {
			// load climate data file manually
			CCM::ClimateDataLoader loader;
			IBK::Path absPath = SVProjectHandler::instance().replacePathPlaceholders(loc.m_climateFilePath);
			try {
				loader.readClimateData(absPath, true);
				m_latitudeInDeg = loader.m_latitudeInDegree;
				m_longitudeInDeg = loader.m_longitudeInDegree;
			}
			catch (IBK::Exception&) {
				QMessageBox::critical(this, QString(), tr("Error reading climate data file '%1'.").arg(QString::fromStdString(absPath.str())));
				return;
			}
		}
	}


	double sunConeDeg = m_ui->lineEditSunCone->value();
	double stepDuration = 3600/m_ui->lineEditSteps->value();

	m_shading->initializeShadingCalculation(loc.m_timeZone,
											loc.m_para[NANDRAD::Location::P_Longitude].get_value("Deg"),
											loc.m_para[NANDRAD::Location::P_Latitude].get_value("Deg"),
											m_startTime,
											(unsigned int)m_durationInSec,
											(unsigned int)stepDuration,
											sunConeDeg );


	// *** compose vectors with obstacles
	for (const VICUS::Surface *s: m_selObstacles)
		selObst.push_back( SH::StructuralShading::ShadingObject(s->m_id,
																s->m_displayName.toStdString(),
																VICUS::INVALID_ID,
																IBKMK::Polygon3D(s->geometry().polygon3D().vertexes() ),
																convertVicus2IBKMKPolyVector(s->geometry().holes()),
																true) );

	// *** compose vector with selected surfaces
	std::vector<unsigned int> surfaceIDs; // holds IDs of calculated surfaces
	std::vector<std::string> surfaceDisplayNames; // holds corresponding display names of calculated surfaces

	// hold reference to project
	VICUS::Project &p = const_cast<VICUS::Project &>(project());
	SVSettings::instance().m_db.updateEmbeddedDatabase(p);
	p.updatePointers();

	// double minArea = 0.1; //1e-4;

	unsigned int skippedSmallSurfaces = 0;
	unsigned int skippedSurfaceWithoutBCtoSky = 0;

	for (const VICUS::Surface *s: m_selSurfaces) {

		if( !s->geometry().isValid() )
			continue;

		if ( s->m_componentInstance == nullptr )
			continue;  // skip invalid surfaces - surfaces without component are not computed in calculation and thus do not require shading factors

		if (const_cast<VICUS::Surface *>(s)->geometry().area() < VICUS::MIN_AREA_FOR_EXPORTED_SURFACES) {
			++skippedSmallSurfaces;
			continue; // we skip small surfaces, since they are not exported to NANDRAD anyway
		}

		// we want to take only surface connected to ambient, that means, the associated component instance
		// must have one zone with ID 0 assigned
		bool hasSideAValidId = s->m_componentInstance->m_idSideASurface != VICUS::INVALID_ID;
		if (hasSideAValidId && s->m_componentInstance->m_idSideBSurface != VICUS::INVALID_ID)
			continue; // skip inside constructions

		// check if this is an surface to outside air and not to ground
		const VICUS::Component * comp = VICUS::element(p.m_embeddedDB.m_components, s->m_componentInstance->m_idComponent);

		// NOTE: we only include a surface in the calculation check, if it is not a ground surfaces.
		//       Hence, if the surface has no component, or not a BC assigned, we skip the surface.

		// find component and boundary condition to check if this is a surface with ground contact
		if (comp != nullptr) {
			unsigned int bbId;
			if (hasSideAValidId)
				bbId = comp->m_idSideBBoundaryCondition;
			else
				bbId = comp->m_idSideABoundaryCondition;

			if (bbId == VICUS::INVALID_ID)
				continue; // adiabatic surface, skipping this is intentional and does not need to pop-up a warning message

			const VICUS::BoundaryCondition *bb = VICUS::element(project().m_embeddedDB.m_boundaryConditions, bbId);
			if (bb == nullptr) {
				// no BB assigned (adiabatic surface?) or BBID invalid
				++skippedSurfaceWithoutBCtoSky;
				continue;
			}
			else {
				// does the surface face the outside?
				if (bb->m_heatConduction.m_otherZoneType != VICUS::InterfaceHeatConduction::OZ_Standard)
				{
					// no, inside or connected to constant zone... skip
					++skippedSurfaceWithoutBCtoSky;
					continue;
				}
			}
		}
		else {
			// no component assigned, skip
			++skippedSurfaceWithoutBCtoSky;
			continue;
		}


		IBKMK::Polygon3D poly = s->geometry().polygon3D();
		const IBKMK::Polygon3D obstaclePoly = s->geometry().polygon3D().vertexes();
		if(m_geometryType == Extruded){

			double totalThickness = 0;
			// calculatwe the total thickness of the corresponding construction of the component
			const VICUS::Construction * construction = VICUS::element(project().m_embeddedDB.m_constructions, comp->m_idConstruction);
			for (const VICUS::MaterialLayer & layer : construction->m_materialLayers) {
				// add all the thicknesses of the different MaterialLayers in meters
				totalThickness += layer.m_thickness.get_value("m");
			}

			// modify the surface by extruding all surface vertexes
			poly.translate(s->geometry().normal()*totalThickness);

			// add additional orthogonal surfaces for all edges
			for(unsigned i = 0; i < obstaclePoly.vertexes().size(); i++){
				std::vector<IBKMK::Vector3D> additionalSurface (4);

				// index 0 & 1 are the original points
				// index 2 & 3 are the shifted points
				additionalSurface[0] = obstaclePoly.vertexes()[i];
				additionalSurface[1] = obstaclePoly.vertexes()[(i+1) % obstaclePoly.vertexes().size()];
				additionalSurface[2] = additionalSurface[1] + s->geometry().normal()*totalThickness;
				additionalSurface[3] = additionalSurface[0] + s->geometry().normal()*totalThickness;

				// debugPolygonPoints(QString("SIDE OBSTACLE %1: ").arg(i), IBKMK::Polygon3D(additionalSurface));

				//add this surface to the obstacles
				selObst.push_back( SH::StructuralShading::ShadingObject(SH::SIDE_SURFACE_ID,
																		QString("%1 - side-surface").arg(s->m_displayName).toStdString(),
																		VICUS::INVALID_ID,
																		IBKMK::Polygon3D(additionalSurface),
																		std::vector<IBKMK::Polygon2D>(),
																		true) );
			}

		}

		// debugPolygonPoints(QString("SURFACE '%1'").arg(s->m_displayName), IBKMK::Polygon3D(poly));

		// we compute shading factors for this surface
		selSurf.push_back( SH::StructuralShading::ShadingObject(s->m_id,
																s->m_displayName.toStdString(),
																VICUS::INVALID_ID,
																poly,
																convertVicus2IBKMKPolyVector(s->geometry().holes()),
																s->m_parent == nullptr) );
		surfaceIDs.push_back(s->m_id);
		surfaceDisplayNames.push_back(s->m_displayName.toStdString());

		// debugPolygonPoints(QString("OBSTACLE '%1'").arg(s->m_displayName), IBKMK::Polygon3D(poly));

		// Mind: surface planes may also shade other surfaces
		selObst.push_back( SH::StructuralShading::ShadingObject(s->m_id,
																s->m_displayName.toStdString(),
																VICUS::INVALID_ID,
																poly,
																convertVicus2IBKMKPolyVector(s->geometry().holes()),
																s->m_parent == nullptr) );
	}

	if (skippedSmallSurfaces != 0) {
		SVSettings::instance().showDoNotShowAgainMessage(this, "shading-calculation-skipped-small-surfaces", QString(),
														 tr("%1 surfaces were skipped, because their surface area was below %2 m2.")
														 .arg(skippedSmallSurfaces).arg(VICUS::MIN_AREA_FOR_EXPORTED_SURFACES));
	}

	// not a single surface left?
	if (selSurf.empty()) {
		QMessageBox::critical(this, QString(), tr("All surfaces have been filtered out because of invalid/missing components or boundary conditions, "
												  "or because they are not facing the sky. Please check your model!"));
		return;
	}

	// some surfaces skipped because of missing/invalid BC?
	if (skippedSurfaceWithoutBCtoSky != 0) {
		SVSettings::instance().showDoNotShowAgainMessage(this, "shading-calculation-skipped-inside-surfaces", QString(),
														 tr("%1 surfaces were skipped, because they have no component assignment or have invalid boundary conditions assigned.")
														 .arg(skippedSmallSurfaces).arg(VICUS::MIN_AREA_FOR_EXPORTED_SURFACES));
	}

	// *** compose vector with selected sub-surfaces
	for (const VICUS::SubSurface *ss: m_selSubSurfaces) {

		if ( ss->m_subSurfaceComponentInstance == nullptr )
			continue;  // skip invalid surfaces - surfaces without component are not computed in calculation and thus do not require shading factors

		if ( ss->m_parent == nullptr ) // no parent; should not be possible but checked anyway
			continue;

		const VICUS::Surface *s = dynamic_cast<const VICUS::Surface*>(ss->m_parent);
		Q_ASSERT(s != nullptr);

		const IBKMK::Vector3D &offset3D = s->geometry().offset();

		const IBKMK::Vector3D &localX = s->geometry().localX();
		const IBKMK::Vector3D &localY = s->geometry().localY();


		//		qDebug() << "======================================================";
		//		qDebug() << QString::fromStdString(ss->m_displayName.toStdString());

		// we need to calculate the 3D Points of the Sub Surface
		std::vector<IBKMK::Vector3D> subSurf3D;
		for (unsigned int i=0; i<ss->m_polygon2D.vertexes().size(); ++i) {
			const IBKMK::Vector2D &vertex = ss->m_polygon2D.vertexes()[i];
			subSurf3D.push_back(offset3D + localX*vertex.m_x + localY*vertex.m_y);
			//			qDebug() << i << "\t" << subSurf3D[i].m_x << "\t" << subSurf3D[i].m_y << "\t" << subSurf3D[i].m_z;
		}

		if(m_geometryType == Extruded){

			SVSettings::instance().showDoNotShowAgainMessage(this, "shading-calculation-extrusion-hint", QString(),
															 tr("Sub-surfaces are currently moved to the middle of the parent surface component. "
																"Later there will be a setting in sub-surface database.") );

			const VICUS::Component * comp = VICUS::element(p.m_embeddedDB.m_components, s->m_componentInstance->m_idComponent);
			if(comp != nullptr){
				double totalThickness = 0;
				// calculate the total thickness of the corresponding construction of the component
				const VICUS::Construction * construction = VICUS::element(project().m_embeddedDB.m_constructions, comp->m_idConstruction);
				for (const VICUS::MaterialLayer & layer : construction->m_materialLayers) {
					// add all the thicknesses of the different MaterialLayers in meters
					totalThickness += layer.m_thickness.get_value("m");
				}

				// extrude the window vertexes by half of the total thickness
				for( IBKMK::Vector3D &v3D : subSurf3D){
					// mind for now we take only half of the component thickness
					v3D += s->geometry().normal()*0.5*totalThickness;
				}

				// debugPolygonPoints(QString("Sub-Surf %1: ").arg(ss->m_displayName), subSurf3D);

				if(!useClipping) {
					// add additional orthogonal surfaces for all edges of the window
					for(unsigned i = 0; i < subSurf3D.size(); i++){
						std::vector<IBKMK::Vector3D> additionalSurface (4);

						// index 0 & 1 are the original points
						// index 2 & 3 are the shifted points
						additionalSurface[0] = subSurf3D[i];
						additionalSurface[1] = subSurf3D[(i+1) % subSurf3D.size()];
						additionalSurface[2] = additionalSurface[1] + s->geometry().normal()*totalThickness*0.5;
						additionalSurface[3] = additionalSurface[0] + s->geometry().normal()*totalThickness*0.5;

						// debugPolygonPoints(QString("WINDOW SIDE OBSTACLE %1: ").arg(i), IBKMK::Polygon3D(additionalSurface));

						//add this surface to the obstacles
						selObst.push_back( SH::StructuralShading::ShadingObject(SH::SIDE_SURFACE_ID,
																				QString("%1 - side-surface").arg(ss->m_displayName).toStdString(),
																				ss->m_id,
																				IBKMK::Polygon3D(additionalSurface),
																				std::vector<IBKMK::Polygon2D>(),
																				true) );
					}
				}
			}
		}

		// we compute shading factors for this surface
		surfaceIDs.push_back(ss->m_id);
		// displaynames of windows use associated surface as parent
		const VICUS::Surface * surf = dynamic_cast<const VICUS::Surface *>(ss->m_parent);
		// if surface has a displayname, use this as prefix
		std::string displayName = surf->m_displayName.toStdString();
		if (!displayName.empty())
			displayName += ".";
		displayName += ss->m_displayName.toStdString();
		surfaceDisplayNames.push_back(displayName);

		selSurf.push_back(SH::StructuralShading::ShadingObject(ss->m_id,
															   ss->m_displayName.toStdString(),
															   s->m_id,
															   IBKMK::Polygon3D(subSurf3D),
															   false) );

		// qDebug() << "Adding sub-surface '" << ss->m_displayName << "'";
	}

	m_shading->setGeometry(selSurf, selObst);

	QProgressDialog progressDialog(tr("Calculate shading factors"), tr("Abort"), 0, 100, this);
	progressDialog.setValue(0);
	progressDialog.setMinimumDuration(1000);

	ShadingCalculationProgress progressNotifyer;
	progressNotifyer.m_dlg = &progressDialog;
	progressNotifyer.m_startTime = QTime::currentTime();

	// *** compute shading ***

	SVProjectHandler &prj = SVProjectHandler::instance();
	QDir projectDir = QFileInfo(prj.projectFile()).dir();

	double gridSize = m_ui->lineEditGridSize->value();
	m_shading->calculateShadingFactors(&progressNotifyer, gridSize, useClipping, IBK::Path(projectDir.absolutePath().toStdString()));

	if (progressNotifyer.m_aborted) {
		QMessageBox::information(this, QString(), tr("Calculation of shading factors was aborted."));
		return;
	}

	progressDialog.hide();

	OutputType outputType = (OutputType)m_ui->comboBoxFileType->currentIndex();
	IBK::Path exportFile;
	// remove any existing shading files with the same name

	std::string exportFileBaseName = projectDir.absoluteFilePath(m_shadingFactorBaseName).toStdString();
	if (IBK::Path(exportFileBaseName + ".tsv").exists())
		IBK::Path::remove(IBK::Path(exportFileBaseName + ".tsv"));
	if (IBK::Path(exportFileBaseName + ".d6o").exists())
		IBK::Path::remove(IBK::Path(exportFileBaseName + ".d6o"));
	if (IBK::Path(exportFileBaseName + ".d6b").exists())
		IBK::Path::remove(IBK::Path(exportFileBaseName + ".d6b"));

	switch ( outputType ) {
	case TsvFile : {
		exportFile = IBK::Path( exportFileBaseName + ".tsv");
		m_shading->writeShadingFactorsToTSV(exportFile, surfaceIDs, surfaceDisplayNames);
	} break;
	case D6oFile : {
		exportFile = IBK::Path( exportFileBaseName + ".d6o" );
		m_shading->writeShadingFactorsToDataIO(exportFile, surfaceIDs, surfaceDisplayNames, false);
	} break;
	case D6bFile : {
		exportFile = IBK::Path( exportFileBaseName + ".d6b" );
		m_shading->writeShadingFactorsToDataIO(exportFile, surfaceIDs, surfaceDisplayNames, true);
	} break;
	}
	QMessageBox::information(this, QString(), tr("Calculated shading factors have been saved to '%1'.").arg(QString::fromStdString(exportFile.str())));

	// Updates latest shading file meta info
	updateShadingFileName();

	// update the shading file infos
	setPreviousSimulationFileValues();
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


void SVSimulationShadingOptions::updateShadingFileName() {
	SVProjectHandler &prj = SVProjectHandler::instance();

	QString projectName = QFileInfo(prj.projectFile()).completeBaseName();
	m_shadingFactorBaseName = projectName + "_shadingFactors";

	QString shadingFileName = m_shadingFactorBaseName;
	OutputType outputType = (OutputType)m_ui->comboBoxFileType->currentIndex();
	switch ( outputType ) {
		case TsvFile : shadingFileName += ".tsv"; break;
		case D6oFile : shadingFileName += ".d6o"; break;
		case D6bFile : shadingFileName += ".d6b"; break;
	}

	m_ui->lineEditShadingFactorFilename->setText(shadingFileName);
}


void SVSimulationShadingOptions::on_comboBoxFileType_currentIndexChanged(int /*index*/) {
	updateShadingFileName();
}


void SVSimulationShadingOptions::on_pushButtonDeletePreviousShadingFile_clicked() {
	SVProjectHandler &prj = SVProjectHandler::instance();
	QDir projectDir = QFileInfo(prj.projectFile()).dir();

	//remove all the previous shading file
	std::string exportFileBaseName = projectDir.absoluteFilePath(m_shadingFactorBaseName).toStdString();
	if (IBK::Path(exportFileBaseName + ".tsv").exists())
		IBK::Path::remove(IBK::Path(exportFileBaseName + ".tsv"));
	if (IBK::Path(exportFileBaseName + ".d6o").exists())
		IBK::Path::remove(IBK::Path(exportFileBaseName + ".d6o"));
	if (IBK::Path(exportFileBaseName + ".d6b").exists())
		IBK::Path::remove(IBK::Path(exportFileBaseName + ".d6b"));

	// to display the changes in the ui
	setPreviousSimulationFileValues();
}


void SVSimulationShadingOptions::on_radioButtonFlatGeometry_toggled(bool isFlatType) {
	if(isFlatType)
		m_geometryType = Flat;
	else
		m_geometryType = Extruded;
}


void SVSimulationShadingOptions::on_radioButtonRayTracing_toggled(bool isRayTracing) {
	m_ui->lineEditGridSize->setVisible(isRayTracing);
	m_ui->labelGridSize->setVisible(isRayTracing);
}

