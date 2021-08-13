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
#include <QTextCodec>

#include "SVProjectHandler.h"
#include "SVSettings.h"

#include <EP_Project.h>
#include <EP_IDFParser.h>

#include <IBKMK_3DCalculations.h>

SVImportIDFDialog::SVImportIDFDialog(QWidget *parent) :
	QDialog(parent),
	m_idfProject(new EP::Project),
	m_ui(new Ui::SVImportIDFDialog)
{
	m_ui->setupUi(this);

	QList<QByteArray> codecs = QTextCodec::availableCodecs();
	std::sort(codecs.begin(), codecs.end());
	for (const QByteArray & b : codecs)
		m_ui->comboBoxEncoding->addItem(QString(b));

	m_ui->comboBoxEncoding->setCurrentText(QTextCodec::codecForLocale()->name());
}


SVImportIDFDialog::~SVImportIDFDialog() {
	delete m_ui;
	delete m_idfProject;
}


SVImportIDFDialog::ImportResults SVImportIDFDialog::import(const QString & fname) {

	// read IDF file

	try {
		EP::IDFParser parser;
		parser.read(IBK::Path(fname.toStdString()));

		m_idfProject->readIDF(parser);
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

	SVDatabase & db = SVSettings::instance().m_db; // readability improvement

	VICUS::Project & vp = m_importedProject; // readability improvement
	vp = VICUS::Project(); // clear any previous data

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

	IBKMK::Vector3D minCoords(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	IBKMK::Vector3D maxCoords(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest());

	// this counter is used to update the progress dialog
	int count = 0;

	QTextCodec * codec = QTextCodec::codecForName(m_ui->comboBoxEncoding->currentText().toLocal8Bit());

	// we start with database components

	// Vector that relates IDF Material definition to VICUS Material IDs -> note that we directly modify SV Database during import
	// to avoid excessive material duplicate generation.
	std::vector<unsigned int> idfMat2VicusMatIDs;

	// TODO : add more string->category matchings (multilang?)
	std::vector<std::pair<QString, VICUS::Material::Category> > categories;
	categories.push_back(std::make_pair("Porenbeton", VICUS::Material::MC_Cementitious));
	categories.push_back(std::make_pair("Beton", VICUS::Material::MC_Cementitious));
	categories.push_back(std::make_pair("Zementestrich", VICUS::Material::MC_Cementitious));

	IBK::IBK_Message("Importing materials...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Material & m : prj.m_materials) {
		// generate VICUS::Material
		VICUS::Material mat;
		QString matName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
		mat.m_displayName.setEncodedString(matName.toStdString() );
		VICUS::KeywordList::setParameter(mat.m_para, "Material::para_t", VICUS::Material::P_Density, m.m_density);
		VICUS::KeywordList::setParameter(mat.m_para, "Material::para_t", VICUS::Material::P_Conductivity, m.m_conductivity);
		VICUS::KeywordList::setParameter(mat.m_para, "Material::para_t", VICUS::Material::P_HeatCapacity, m.m_specHeatCapa);

		for (const std::pair<QString, VICUS::Material::Category> & cat : categories)
			if (matName.startsWith(cat.first))
				mat.m_category = cat.second;

		// color and ID don't matter for now, try to find similar material in DB
		bool found = false;
		for (const std::pair<const unsigned int, VICUS::Material> & dbMat : db.m_materials) {
			if (dbMat.second.equal(&mat) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("  %1 -> using existing material '%2' [%3] \n")
								  .arg("'"+matName.toStdString()+"'", 40, std::ios_base::left)
								  .arg(dbMat.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbMat.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				idfMat2VicusMatIDs.push_back(dbMat.first);
				found = true;
				break;
			}
		}
		if (!found) {
			// no matching material found, add new material to DB
			unsigned int newID = db.m_materials.add(mat);
			IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2 \n")
							  .arg("'"+matName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			idfMat2VicusMatIDs.push_back(newID);
		}
	}

	// For each construction in IDF we store the respective VICUS-construction ID, and also, if the
	// referenced construction is defined in reverse.
	std::vector<std::pair<unsigned int, bool> >  idfConstruction2VicusConIDs;

	IBK::IBK_Message("Importing constructions...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Construction & m : prj.m_constructions) {
		// generate VICUS::ConstructionType
		VICUS::Construction con;
		QString conName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
		con.m_displayName.setEncodedString(conName.toStdString() );

		try {

			// process all referenced materials and lookup matching VICUS material IDs
			for (const std::string & matLay : m.m_layers) {
				// find material by name
				unsigned int i=0;
				for (;i<prj.m_materials.size(); ++i)
					if (prj.m_materials[i].m_name == matLay)
						break;
				if (i == prj.m_materials.size()) {
					// also convert names in error message
					throw IBK::Exception(IBK::FormatString("Material '%1' referenced from construction '%2' is not defined in IDF file.")
										 .arg(codec->toUnicode(matLay.c_str()).toStdString()).arg(conName.toStdString()), FUNC_ID);
				}

				// extract size in [m]
				double thickness = prj.m_materials[i].m_thickness;
				con.m_materialLayers.push_back(VICUS::MaterialLayer(thickness, idfMat2VicusMatIDs[i]) );
			}

		} catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			continue; // skip construction
		}

		VICUS::Construction conRev(con);
		std::reverse(conRev.m_materialLayers.begin(), conRev.m_materialLayers.end());

		bool found = false;
		// now check if construction (or its reverse) does already exist in VICUS DB
		// color and ID don't matter for now, try to find similar material in DB
		for (const std::pair<const unsigned int, VICUS::Construction> & dbCon : db.m_constructions) {
			if (dbCon.second.equal(&con) != VICUS::AbstractDBElement::Different) {
				// re-use this construction
				IBK::IBK_Message( IBK::FormatString("  %1 -> using existing construction '%2' [%3] \n")
								  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left)
								  .arg(dbCon.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbCon.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				idfConstruction2VicusConIDs.push_back( std::make_pair(dbCon.first, false) ); // not a reverse construction
				found = true;
				break;
			}
			if (dbCon.second.equal(&conRev) != VICUS::AbstractDBElement::Different) {
				// re-use this construction
				IBK::IBK_Message( IBK::FormatString("  %1 -> using existing construction '%2' [%3] (reversed)\n")
								  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left)
								  .arg(dbCon.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbCon.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				idfConstruction2VicusConIDs.push_back( std::make_pair(dbCon.first, true) ); // a reverse construction
				found = true;
				break;
			}
		}

		if (!found) {
			// no matching construction found, add new to DB
			unsigned int newID = db.m_constructions.add(con);
			IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2 \n")
							  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			idfConstruction2VicusConIDs.push_back( std::make_pair(newID, false) ); // not a reverse construction
		}
	}


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
		surf.polygon3D().enlargeBoundingBox(minCoords, maxCoords);

		surf.initializeColorBasedOnInclination();
		surf.m_color = surf.m_displayColor;
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


	// *** ShadingBuildingDetailed ***

	//import all building surface detailed -> opaque surfaces
	for (const EP::ShadingBuildingDetailed &sh : prj.m_shadingBuildingDetailed) {
		++count;
		if (progressTimer.elapsed() > 100) {
			dlg.setValue(count);
			if (dlg.wasCanceled())
				throw IBK::Exception("Import canceled.", FUNC_ID);
			progressTimer.start();
		}

		VICUS::Surface surf;
		surf.m_id = surf.uniqueID();
		surf.m_displayName = QString::fromStdString(sh.m_name);
		surf.setPolygon3D( VICUS::Polygon3D( sh.m_polyline ) );
		surf.polygon3D().enlargeBoundingBox(minCoords, maxCoords);
		surf.m_color = surf.m_displayColor = QColor("#67759d");

		vp.m_plainGeometry.push_back(surf);
	}


	// TODO : Windows

//	// finally initialize subsurface colors
//	for (VICUS::SubSurfaceComponentInstance & sub : vp.m_subSurfaceComponentInstances) {
//		VICUS::SubSurfaceComponent * subComp = db.m_subSurfaceComponents[sub.m_idSubSurfaceComponent];
//		subComp
//	}


	// set site properties based on extends of imported geometry
	double maxDist = 100;
	maxDist = std::max(maxDist, maxCoords.m_x - minCoords.m_x);
	maxDist = std::max(maxDist, maxCoords.m_y - minCoords.m_y);
	maxDist = std::max(maxDist, maxCoords.m_z - minCoords.m_z);
	vp.m_viewSettings.m_farDistance = maxDist*4;
	vp.m_viewSettings.m_gridWidth = maxDist;
	if (maxDist < 10)
		vp.m_viewSettings.m_gridSpacing = 0.1;
	else if (maxDist < 100)
		vp.m_viewSettings.m_gridSpacing = 1;
	else if (maxDist < 500)
		vp.m_viewSettings.m_gridSpacing = 10;
	else if (maxDist < 1000)
		vp.m_viewSettings.m_gridSpacing = 50;
	else
		vp.m_viewSettings.m_gridSpacing = 100;
}


bool SVImportIDFDialog::doImport() {
	try {
		// now transfer data to temporary VICUS project structure.
		transferData(*m_idfProject);
	}
	catch (IBK::Exception & ex) {
		if (ex.what() == QString("Import canceled."))
			return false;
		ex.writeMsgStackToError();
		QMessageBox::critical(this, tr("IDF Import"), tr("There were critical errors during the import (invalid IDF file), see application log for details."));
		return false;
	}
	return true;
}


void SVImportIDFDialog::on_pushButtonReplace_clicked() {
	if (!doImport()) return;

	m_returnCode = ReplaceProject;
	accept();
}


void SVImportIDFDialog::on_pushButtonMerge_clicked() {
	if (!doImport()) return;

	m_returnCode = MergeProjects;
	accept();
}


