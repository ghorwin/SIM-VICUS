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

#include "SVImportIDFDialog.h"
#include "ui_SVImportIDFDialog.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QElapsedTimer>

#include "SVProjectHandler.h"

#include <EP_Project.h>
#include <EP_IDFParser.h>

#include <IBKMK_3DCalculations.h>

SVImportIDFDialog::SVImportIDFDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVImportIDFDialog)
{
	m_ui->setupUi(this);
}


SVImportIDFDialog::~SVImportIDFDialog() {
	delete m_ui;
}


SVImportIDFDialog::ImportResults SVImportIDFDialog::import(const QString & fname) {

	// read IDF file

	try {
		EP::IDFParser parser;
		parser.read(IBK::Path(fname.toStdString()));

		EP::Project prj;
		prj.readIDF(parser);

		// now transfer data to temporary VICUS project structure.
		m_importedProject = VICUS::Project(); // clear data from previous import
		transferData(prj);
	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical((QWidget*)parent(), tr("Import error"), tr("Error parsing IDF file:\n%1").arg(ex.what()));
		return ImportCancelled;
	}

	// if successful, show dialog

	// merge project is only active if we have a project

	m_ui->pushButtonMerge->setEnabled( SVProjectHandler::instance().isValid() );

	int res = exec();
	if (res == QDialog::Rejected)
		return ImportCancelled;

	// TODO : apply coordinate shift to imported building geometry


	return m_returnCode;

}


void SVImportIDFDialog::transferData(const EP::Project & prj) {
	FUNCID(SVImportIDFDialog::transferData);

	// TODO : Dirk, error handling concept
	//        - decide which errors are critical and cause abort of import
	//        - which errors can just be ignored (resulting in an incomplete import)
	//
	// If invalid parameter sets are found, and are skipped during import, there should be
	// messages collected for each case and shown to the user after the import to inform about
	// import problems. How do we do this? Stringlist with error messages shows as table afterwards?
	// Single string were errors are appended to, shown in an error message box?


	VICUS::Project & vp = m_importedProject; // readability improvement
	vp.m_buildings.resize(1);
	vp.m_buildings[0].m_buildingLevels.resize(1);
	vp.m_buildings[0].m_id = vp.m_buildings[0].uniqueID();
	vp.m_buildings[0].m_displayName = tr("Imported IDF Building Geometry");

	VICUS::BuildingLevel & bl = vp.m_buildings[0].m_buildingLevels[0];
	bl.m_id = bl.uniqueID();
	bl.m_displayName = tr("Default building level");

	QProgressDialog dlg(tr("Importing IDF project"), tr("Abort"), 0, prj.m_zones.size(), this);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.setValue(0);
	qApp->processEvents();

	QElapsedTimer progressTimer;
	progressTimer.start();

	// this counter is used to update the progress dialog
	int count = 0;

	// *** Zone ***

	// a map that relates zone name to index in the zones map
	std::map<std::string, unsigned int>	mapZoneNameToIdx;
	for (const EP::Zone & z : prj.m_zones) {
		++count;
		if (progressTimer.elapsed() > 100) {
			dlg.setValue(count);
			if (dlg.wasCanceled())
				throw IBK::Exception("Import canceled.", FUNC_ID);
			progressTimer.start();
		}

		VICUS::Room r;
		r.m_id = r.uniqueID();
		r.m_displayName = QString::fromStdString(z.m_name);

		// remember zone name - id association
		if (mapZoneNameToIdx.find(z.m_name) != mapZoneNameToIdx.end())
			throw IBK::Exception(IBK::FormatString("Duplicate zone ID name '%1'").arg(z.m_name), FUNC_ID);

		// transfer attributes

		// ceiling height is not taken into account
		if(z.m_floorArea > 0)
			r.m_para[VICUS::Room::P_Area].set("Area", z.m_floorArea, "m2" );
		if(z.m_volume > 0)
			r.m_para[VICUS::Room::P_Volume].set("Volume", z.m_volume, "m3" );

		// add zone and remember its index
		mapZoneNameToIdx[z.m_name] = bl.m_rooms.size();
		bl.m_rooms.push_back(r);
	}


	// *** BuildingSurfaceDetailed ***

	// a map that relates a bsd-ID name (display name of a surface) to its ID
	std::map<std::string, unsigned int> mapBsdNameIDmap;

	//import all building surface detailed -> opaque surfaces
	for (const EP::BuildingSurfaceDetailed &bsd : prj.m_bsd) {
		++count;
		if (progressTimer.elapsed() > 100) {
			dlg.setValue(count);
			if (dlg.wasCanceled())
				throw IBK::Exception("Import canceled.", FUNC_ID);
			progressTimer.start();
		}

		const auto & zoneIt = mapZoneNameToIdx.find(bsd.m_zoneName);
		if (zoneIt == mapZoneNameToIdx.end())
			throw IBK::Exception(IBK::FormatString("Zone name '%1' does not exist, which is referenced in "
												   "Building Surface Detailed '%2'").arg(bsd.m_zoneName)
													.arg(bsd.m_name), FUNC_ID);
		unsigned idx = zoneIt->second;

		VICUS::Surface surf;
		surf.m_id = surf.uniqueID();
		surf.m_displayName = QString::fromStdString(bsd.m_name);
		surf.setPolygon3D( VICUS::Polygon3D( bsd.m_polyline ) );

		surf.updateColor();
		bl.m_rooms[idx].m_surfaces.push_back(surf);

		mapBsdNameIDmap[bsd.m_name] = surf.m_id;
	}

	// add surfaces windows, doors, ...

	// *** FenestrationSurfaceDetailed ***

	// we need to collect a vector of subsurfaces for each surface to add to
	// also, we need to create subsurface components and reference these

	for (const EP::FenestrationSurfaceDetailed &fsd : prj.m_fsd) {
		++count;
		if (progressTimer.elapsed() > 100) {
			dlg.setValue(count);
			if (dlg.wasCanceled())
				throw IBK::Exception("Import canceled.", FUNC_ID);
			progressTimer.start();
		}

		// look up surface that this fenestration belongs to
		const auto & bsd = mapBsdNameIDmap.find(fsd.m_bsdName);
		if (bsd == mapBsdNameIDmap.end()) {
			throw IBK::Exception(IBK::FormatString("Building Surface Detailed name '%1' does not exist, which is "
												   "referenced in Fenestration Surface Detailed '%2'")
								 .arg(fsd.m_bsdName).arg(fsd.m_name), FUNC_ID);
		}

		VICUS::Surface * surf = vp.surfaceByID(bsd->second);
		Q_ASSERT(surf != nullptr);
		// we can only import a subsurface, if the surface itself has a valid polygon
		if (!surf->geometry().isValid()) {
			qDebug() << "Invalid surface" << surf->m_displayName << ", cannot add subsurfaces!";
			continue;
		}

		// now convert the 3D polyline into projected 2D coordinates

		std::vector<IBKMK::Vector2D> subSurfaceVertexes;
		for (unsigned int i=0; i<fsd.m_polyline.size(); ++i) {
			const IBKMK::Vector3D & x0 = fsd.m_polyline[i];
			double x,y;
			if (!IBKMK::planeCoordinates(surf->geometry().offset(), surf->geometry().localX(), surf->geometry().localY(), x0, x, y, 1e-4)) {
				qDebug() << "Invalid point of subsurface polygon!";
				subSurfaceVertexes.clear();
				break;
			}
			subSurfaceVertexes.push_back(IBKMK::Vector2D(x,y));
		}
		if (subSurfaceVertexes.empty())
			continue;

		// set the subsurface
		VICUS::SubSurface subSurf;
		subSurf.m_id = subSurf.uniqueID();
		subSurf.m_displayName = QString::fromStdString(fsd.m_name);
		subSurf.m_polygon2D.setVertexes(subSurfaceVertexes);
		if (!subSurf.m_polygon2D.isValid()) {
			qDebug() << "Invalid subsurface polygon!";
			continue;
		}
		std::vector<VICUS::SubSurface> subs = surf->subSurfaces();
		subs.push_back(subSurf);
		surf->setSubSurfaces(subs);
	}
}

void SVImportIDFDialog::on_pushButtonReplace_clicked() {
	m_returnCode = ReplaceProject;
	accept();
}

void SVImportIDFDialog::on_pushButtonMerge_clicked() {
	m_returnCode = MergeProjects;
	accept();
}
