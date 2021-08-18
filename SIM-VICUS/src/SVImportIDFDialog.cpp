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
#include <QPlainTextEdit>
#include <QTextCodec>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVMessageHandler.h"
#include "SVStyle.h"

#include <EP_Project.h>
#include <EP_IDFParser.h>

#include <VICUS_utilities.h>

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

	SVStyle::instance().formatPlainTextEdit(m_ui->plainTextEdit);
}


SVImportIDFDialog::~SVImportIDFDialog() {
	delete m_ui;
	delete m_idfProject;
}


SVImportIDFDialog::ImportResults SVImportIDFDialog::import(const QString & fname) {
	FUNCID(SVImportIDFDialog::import);

	// Create message handler instance -> redirect all IBK::IBK_message output to message handler
	// at end of function, this object get's destroyed and resets the default message handler
	SVImportMessageHandler msgHandler(this, m_ui->plainTextEdit);

	// read IDF file
	try {
		EP::IDFParser parser;
		parser.read(IBK::Path(fname.toStdString()));

		IBK::IBK_Message("Parsing IDF...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		m_idfProject->readIDF(parser);


	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical((QWidget*)parent(), tr("Import error"), tr("Error parsing IDF file:\n%1").arg(ex.what()));
		return ImportCancelled;
	}

	// if successful, show dialog
	m_ui->pushButtonMerge->setEnabled(false);
	m_ui->pushButtonReplace->setEnabled(false);

	int res = exec();
	if (res == QDialog::Rejected)
		return ImportCancelled;

	return m_returnCode;
}


void updateProgress(QProgressDialog * dlg, QElapsedTimer & progressTimer) {
	const unsigned int PROGRESS_TIMER_MSEC_DELAY = 200;
	if (progressTimer.elapsed() > PROGRESS_TIMER_MSEC_DELAY) {
		dlg->setValue(dlg->value()+1);
		if (dlg->wasCanceled())
			throw IBK::Exception("Import canceled.", "[SVImportIDFDialog::transferData]");
		progressTimer.start();
	}
}


void SVImportIDFDialog::transferData(const EP::Project & prj) {
	FUNCID(SVImportIDFDialog::transferData);

	m_ui->plainTextEdit->clear();

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

	QProgressDialog dlg(tr("Importing IDF project"), tr("Abort"), 0, 0, this);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.setValue(0);
	qApp->processEvents();

	QElapsedTimer progressTimer;
	progressTimer.start();

	IBKMK::Vector3D minCoords(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	IBKMK::Vector3D maxCoords(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest());

	QTextCodec * codec = QTextCodec::codecForName(m_ui->comboBoxEncoding->currentText().toLocal8Bit());

	// we start with database components

	// Vector that relates IDF Material definition to VICUS Material IDs -> note that we directly modify SV Database during import
	// to avoid excessive material duplicate generation.
	// We store this as vector and not as map, because we need the index to access IDF material properties later.
	std::vector<unsigned int> idfMat2VicusMatIDs;

	// TODO : add more string->category matchings (multilang?)
	std::vector<std::pair<QString, VICUS::Material::Category> > categories;
	categories.push_back(std::make_pair("Porenbeton", VICUS::Material::MC_Cementitious));
	categories.push_back(std::make_pair("Beton", VICUS::Material::MC_Cementitious));
	categories.push_back(std::make_pair("Zementestrich", VICUS::Material::MC_Cementitious));

	IBK::IBK_Message("\nImporting materials...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Material & m : prj.m_materials) {
		// generate VICUS::Material
		VICUS::Material mat;
		QString matName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
		mat.m_displayName.setEncodedString(matName.toStdString() );
		mat.m_color = SVStyle::randomColor();

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
				IBK::IBK_Message( IBK::FormatString("  %1 -> using existing material '%2' [#%3] \n")
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
			IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2\n")
							  .arg("'"+matName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			idfMat2VicusMatIDs.push_back(newID);
		}
	}

	// For each construction in IDF we store the respective VICUS-construction ID, and also, if the
	// referenced construction is defined in reverse.
	std::vector<std::pair<unsigned int, bool> >  idfConstruction2VicusConIDs;

	IBK::IBK_Message("\nImporting constructions...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Construction & m : prj.m_constructions) {
		// generate VICUS::ConstructionType
		VICUS::Construction con;
		QString conName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
		con.m_displayName.setEncodedString(conName.toStdString() );
		con.m_color = SVStyle::randomColor();

		try {

			// process all referenced materials and lookup matching VICUS material IDs
			for (const std::string & matLay : m.m_layers) {
				// find material by name
				unsigned int i = VICUS::elementIndex(prj.m_materials, matLay);
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
				IBK::IBK_Message( IBK::FormatString("  %1 -> using existing construction '%2' [#%3]\n")
								  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left)
								  .arg(dbCon.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbCon.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				idfConstruction2VicusConIDs.push_back( std::make_pair(dbCon.first, false) ); // not a reverse construction
				found = true;
				break;
			}
			if (dbCon.second.equal(&conRev) != VICUS::AbstractDBElement::Different) {
				// re-use this construction
				IBK::IBK_Message( IBK::FormatString("  %1 -> using existing construction '%2' [#%3] (reversed)\n")
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
			IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2\n")
							  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			idfConstruction2VicusConIDs.push_back( std::make_pair(newID, false) ); // not a reverse construction
		}
	}


	// *** Inside boundary conditions ("surface") ***

	unsigned int bcIDSurface = VICUS::INVALID_ID; // holds the ID of inside surfaces
	{
		VICUS::BoundaryCondition bcSurface;
		QString bcName = tr("Inside surface");
		bcSurface.m_displayName.setEncodedString(bcName.toStdString());
		bcSurface.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::MT_Constant;
		VICUS::KeywordList::setParameter(bcSurface.m_heatConduction.m_para, "InterfaceHeatConduction::para_t", VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 8);
		// TODO : how to set the remaining parameters?


		bool found = false;
		for (const std::pair<const unsigned int, VICUS::BoundaryCondition> & dbBC : db.m_boundaryConditions) {
			if (dbBC.second.equal(&bcSurface) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("\nUsing existing boundary condition '%1' [#%2]\n")
								  .arg(dbBC.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbBC.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				bcIDSurface = dbBC.first;
				found = true;
				break;
			}
		}
		if (!found) {
			// no matching BC found, add new BC to DB
			bcSurface.m_color = SVStyle::randomColor();
			bcIDSurface = db.m_boundaryConditions.add(bcSurface);
			IBK::IBK_Message( IBK::FormatString("\nAdded new boundary condition '%1' with ID #%2\n")
							  .arg(bcName.toStdString()).arg(bcIDSurface), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}

	// *** Ground boundary conditions ("ground") ***

	unsigned int bcIDGround = VICUS::INVALID_ID;
	{
		VICUS::BoundaryCondition bc;
		QString bcName = tr("Ground surface");
		bc.m_displayName.setEncodedString(bcName.toStdString());
		bc.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::MT_Constant;
		bc.m_heatConduction.m_otherZoneType = VICUS::InterfaceHeatConduction::OZ_Constant;
		VICUS::KeywordList::setParameter(bc.m_heatConduction.m_para, "InterfaceHeatConduction::para_t", VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 8);
		// TODO : how to set the remaining parameters?


		bool found = false;
		for (const std::pair<const unsigned int, VICUS::BoundaryCondition> & dbBC : db.m_boundaryConditions) {
			if (dbBC.second.equal(&bc) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("\nUsing existing boundary condition '%1' [#%2]\n")
								  .arg(dbBC.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbBC.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				bcIDGround = dbBC.first;
				found = true;
				break;
			}
		}
		if (!found) {
			// no matching BC found, add new BC to DB
			bc.m_color = SVStyle::randomColor();
			bcIDGround = db.m_boundaryConditions.add(bc);
			IBK::IBK_Message( IBK::FormatString("\nAdded new boundary condition '%1' with ID #%2\n")
							  .arg(bcName.toStdString()).arg(bcIDSurface), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}


	// *** Adiabatic boundary conditions ("adiabatic") ***

	unsigned int bcIDAdiabatic= VICUS::INVALID_ID;
	{
		VICUS::BoundaryCondition bc;
		QString bcName = tr("Adiabatic surface");
		bc.m_displayName.setEncodedString(bcName.toStdString());
		bc.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::NUM_MT;
//		bc.m_heatConduction.m_otherZoneType = VICUS::InterfaceHeatConduction::OZ_Constant;
		VICUS::KeywordList::setParameter(bc.m_heatConduction.m_para, "InterfaceHeatConduction::para_t", VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 0);
		// TODO : how to set the remaining parameters?

		bool found = false;
		for (const std::pair<const unsigned int, VICUS::BoundaryCondition> & dbBC : db.m_boundaryConditions) {
			if (dbBC.second.equal(&bc) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("\nUsing existing boundary condition '%1' [#%2]\n")
								  .arg(dbBC.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbBC.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				bcIDAdiabatic = dbBC.first;
				found = true;
				break;
			}
		}
		if (!found) {
			// no matching BC found, add new BC to DB
			bc.m_color = SVStyle::randomColor();
			bcIDAdiabatic = db.m_boundaryConditions.add(bc);
			IBK::IBK_Message( IBK::FormatString("\nAdded new boundary condition '%1' with ID #%2\n")
							  .arg(bcName.toStdString()).arg(bcIDSurface), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}

	// *** Zone ***

	// a map that relates zone name to index in the VICUS room vector
	std::map<std::string, unsigned int>	mapZoneNameToIdx;
	IBK::IBK_Message("\nImporting zones...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Zone & z : prj.m_zones) {
		updateProgress(&dlg, progressTimer);

		VICUS::Room r;
		r.m_id = r.uniqueID();
		r.m_displayName = codec->toUnicode(z.m_name.c_str()); // Mind text encoding here!

		// remember zone name - id association
		if (mapZoneNameToIdx.find(z.m_name) != mapZoneNameToIdx.end())
			throw IBK::Exception(IBK::FormatString("Duplicate zone ID name '%1'").arg(z.m_name), FUNC_ID);

		// transfer attributes

		// ceiling height is not taken into account
		if (z.m_floorArea > 0)
			r.m_para[VICUS::Room::P_Area].set("Area", z.m_floorArea, "m2" );
		if (z.m_volume > 0)
			r.m_para[VICUS::Room::P_Volume].set("Volume", z.m_volume, "m3" );

		IBK::IBK_Message( IBK::FormatString("  %1 [#%2] (area=%3 m2, volume=%4 m3)\n")
						  .arg("'"+r.m_displayName.toStdString()+"'", 40, std::ios_base::left)
						  .arg(r.m_id)
						  .arg(r.m_para[VICUS::Room::P_Area].value)
						  .arg(r.m_para[VICUS::Room::P_Volume].value), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// add zone and remember its index
		mapZoneNameToIdx[z.m_name] = bl.m_rooms.size();
		bl.m_rooms.push_back(r);
	}



	// *** BuildingSurfaceDetailed ***

	// a map that relates a bsd-ID name (display name of a surface) to its ID
	std::map<std::string, unsigned int> mapBsdNameIDmap;

	// import all building surface detailed -> opaque surfaces
	IBK::IBK_Message("\nImporting surfaces...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::BuildingSurfaceDetailed &bsd : prj.m_bsd) {
		updateProgress(&dlg, progressTimer);

		const auto & zoneIt = mapZoneNameToIdx.find(bsd.m_zoneName);
		if (zoneIt == mapZoneNameToIdx.end())
			throw IBK::Exception(IBK::FormatString("Zone name '%1' does not exist, which is referenced in "
												   "BuildingSurface:Detailed '%2'").arg(bsd.m_zoneName)
													.arg(bsd.m_name), FUNC_ID);
		unsigned idx = zoneIt->second;

		VICUS::Surface surf;
		surf.m_id = surf.uniqueID();
		surf.m_displayName = codec->toUnicode(bsd.m_name.c_str()); // Mind text encoding here!
		surf.setPolygon3D( VICUS::Polygon3D( bsd.m_polyline ) );

		// we can only import a subsurface, if the surface itself has a valid polygon
		if (!surf.geometry().isValid()) {
			IBK::IBK_Message(IBK::FormatString("Invalid geometry of surface %1.")
							 .arg(surf.m_displayName.toStdString()),
							 IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		}

		surf.polygon3D().enlargeBoundingBox(minCoords, maxCoords);

		surf.initializeColorBasedOnInclination();
		surf.m_color = surf.m_displayColor;
		bl.m_rooms[idx].m_surfaces.push_back(surf);

		IBK::IBK_Message( IBK::FormatString("  %3.%1 [#%2]\n")
						  .arg(surf.m_displayName.toStdString())
						  .arg(surf.m_id)
						  .arg(bl.m_rooms[idx].m_displayName.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// remember bsd/surface name - id association
		mapBsdNameIDmap[bsd.m_name] = surf.m_id;
	}


	// *** Components and ComponentInstances ***

	// here we store the IDs of connected surfaces, so that we do not create duplicate inside walls
	std::set<unsigned int> connectedSurfaces;

	// Now the bsd -> surface map is complete.
	// We now create all components and component instances. For that, we loop again over all bsd.
	IBK::IBK_Message("\nImporting components...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::BuildingSurfaceDetailed &bsd : prj.m_bsd) {
		QString bsdName = codec->toUnicode(bsd.m_name.c_str()); // Mind text encoding here!

		// *** Components ***

		// we first create a component with the referenced construction
		VICUS::Component com;
		com.m_displayName.setEncodedString("en:Component-" + bsdName.toStdString());
		// lookup construction
		unsigned int conIdx = VICUS::elementIndex(prj.m_constructions, bsd.m_constructionName);
		if (conIdx == prj.m_constructions.size()) {
			// also convert names in error message
			throw IBK::Exception(IBK::FormatString("Construction '%1' referenced from BSD '%2' is not defined in IDF file.")
								 .arg(codec->toUnicode(bsd.m_constructionName.c_str()).toStdString())
								 .arg(bsdName.toStdString()), FUNC_ID);
		}
		// lookup matching VICUS::Construction ID
		com.m_idConstruction = idfConstruction2VicusConIDs[conIdx].first;
		bool inverted = idfConstruction2VicusConIDs[conIdx].second; // if true, this is the inverted variant of a construction

		// now create boundary conditions
		unsigned int otherSurfaceID = VICUS::INVALID_ID;

		switch (bsd.m_outsideBoundaryCondition) {

			// inside surface?
			case EP::BuildingSurfaceDetailed::OC_Surface : {
				// we require an opposite surface set
				if (bsd.m_outsideBoundaryConditionObject.empty())
					throw IBK::Exception(IBK::FormatString("BSD '%1' of type 'Surface' does not reference outside BC object.")
										 .arg(bsdName.toStdString()), FUNC_ID);
				com.m_idSideABoundaryCondition = bcIDSurface;
				com.m_idSideBBoundaryCondition = bcIDSurface;

				// store ID of other surface
				otherSurfaceID = mapBsdNameIDmap[bsd.m_outsideBoundaryConditionObject];
				IBK_ASSERT(otherSurfaceID != 0);
			} break;

			case EP::BuildingSurfaceDetailed::OC_Ground : {
				com.m_idSideABoundaryCondition = bcIDSurface;
				com.m_idSideBBoundaryCondition = bcIDGround;
			} break;

			case EP::BuildingSurfaceDetailed::OC_Outdoors : {
				com.m_idSideABoundaryCondition = bcIDSurface;
			} break;

			case EP::BuildingSurfaceDetailed::OC_Adiabatic : {
				com.m_idSideABoundaryCondition = bcIDSurface;
				com.m_idSideBBoundaryCondition = bcIDAdiabatic;
			} break;

			case EP::BuildingSurfaceDetailed::NUM_OC : ; // just to make compiler happy
		}


		// if inverted, swap BC assignments
		if (inverted)
			std::swap(com.m_idSideABoundaryCondition, com.m_idSideBBoundaryCondition);

		unsigned int comId = VICUS::INVALID_ID;
		// check, if we have such a component already
		bool found = false;
		for (const std::pair<const unsigned int, VICUS::Component> & dbElement : db.m_components) {
			if (dbElement.second.equal(&com) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("  -> using existing component '%1' [#%2]\n")
								  .arg(dbElement.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbElement.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				comId = dbElement.first;
				found = true;
				break;
			}
		}
		if (!found) {
			// no matching component found, add new to DB
			com.m_color = SVStyle::randomColor();
			comId = db.m_components.add(com);
			IBK::IBK_Message( IBK::FormatString("  -> added new component '%1' [#%2]\n")
							  .arg(com.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(comId), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}

		// check that the current surface hasn't been connected, yet
		unsigned int surfID = mapBsdNameIDmap[bsd.m_name];
		IBK_ASSERT(surfID != 0);
		// skip interface generation, if surface has been handled already
		if (connectedSurfaces.find(surfID) != connectedSurfaces.end()) {
			IBK::IBK_Message( IBK::FormatString("    skipped '%1' [#%2] (has been connected already)\n")
					.arg(codec->toUnicode(bsd.m_name.c_str()).toStdString()).arg(surfID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			continue;
		}

		connectedSurfaces.insert(surfID);
		if (otherSurfaceID != VICUS::INVALID_ID)
			connectedSurfaces.insert(otherSurfaceID);

		// now create a new component instance
		VICUS::ComponentInstance ci;
		ci.m_id = VICUS::uniqueId(vp.m_componentInstances);
		ci.m_idSideASurface = surfID;
		ci.m_idSideBSurface = otherSurfaceID;

		if (inverted)
			std::swap(ci.m_idSideASurface, ci.m_idSideBSurface);

		ci.m_idComponent = comId;

		if (otherSurfaceID != VICUS::INVALID_ID)
			IBK::IBK_Message( IBK::FormatString("    '%1' [#%2] connected to surface '%3' [#%4]\n")
				.arg(codec->toUnicode(bsd.m_name.c_str()).toStdString()).arg(surfID)
				.arg(codec->toUnicode(bsd.m_outsideBoundaryConditionObject.c_str()).toStdString()).arg(otherSurfaceID),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		else {
			// TODO : distinguish between ambient and ground
			IBK::IBK_Message( IBK::FormatString("    '%1' [#%2] connected to outside\n")
				.arg(codec->toUnicode(bsd.m_name.c_str()).toStdString()).arg(surfID),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
		vp.m_componentInstances.push_back(ci);
	}




	// add surfaces windows, doors, ...

	// TODO : review code below


	// *** FenestrationSurfaceDetailed ***

	// we need to collect a vector of subsurfaces for each surface to add to
	// also, we need to create subsurface components and reference these

	IBK::IBK_Message("\nImporting windows (sub-surfaces)...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::FenestrationSurfaceDetailed &fsd : prj.m_fsd) {
		updateProgress(&dlg, progressTimer);

		QString fsdName = codec->toUnicode(fsd.m_name.c_str()); // Mind text encoding here!
		QString bsdName = codec->toUnicode(fsd.m_bsdName.c_str()); // Mind text encoding here!
		// look up surface that this fenestration belongs to
		const auto & bsd = mapBsdNameIDmap.find(fsd.m_bsdName);
		if (bsd == mapBsdNameIDmap.end()) {
			throw IBK::Exception(IBK::FormatString("BuildingSurface:Detailed name '%1' does not exist, which is "
												   "referenced in FenestrationSurfaceDetailed '%2'.")
								 .arg(bsdName.toStdString()).arg(fsdName.toStdString()), FUNC_ID);
		}

		VICUS::Surface * surf = vp.surfaceByID(bsd->second);
		IBK_ASSERT(surf != nullptr);

		// now convert the 3D polyline into projected 2D coordinates

		std::vector<IBKMK::Vector2D> subSurfaceVertexes;
		for (unsigned int i=0; i<fsd.m_polyline.size(); ++i) {
			const IBKMK::Vector3D & x0 = fsd.m_polyline[i];
			double x,y;
			if (!IBKMK::planeCoordinates(surf->geometry().offset(), surf->geometry().localX(), surf->geometry().localY(), x0, x, y, 1e-4)) {
				IBK::IBK_Message(IBK::FormatString("Invalid point of sub-surface polygon of FenestrationSurfaceDetailed '%1'.")
								 .arg(fsdName.toStdString()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
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
			IBK::IBK_Message(IBK::FormatString("Invalid sub-surface polygon of FenestrationSurfaceDetailed '%1'.")
							 .arg(fsdName.toStdString()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			continue;
		}
		std::vector<VICUS::SubSurface> subs = surf->subSurfaces();
		subs.push_back(subSurf);
		surf->setSubSurfaces(subs);

		IBK::IBK_Message( IBK::FormatString("  %1.%2 [#%3]\n")
						  .arg(surf->m_displayName.toStdString())
						  .arg(subSurf.m_displayName.toStdString())
						  .arg(subSurf.m_id),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}


	// *** ShadingBuildingDetailed ***

	IBK::IBK_Message("\nShading geometry (ShadingBuildingDetailed)...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::ShadingBuildingDetailed &sh : prj.m_shadingBuildingDetailed) {
		updateProgress(&dlg, progressTimer);

		VICUS::Surface surf;
		surf.m_id = surf.uniqueID();
		surf.m_displayName = codec->toUnicode(sh.m_name.c_str()); // Mind text encoding here!
		surf.setPolygon3D( VICUS::Polygon3D( sh.m_polyline ) );
		surf.polygon3D().enlargeBoundingBox(minCoords, maxCoords);
		surf.m_color = surf.m_displayColor = QColor("#67759d");

		vp.m_plainGeometry.push_back(surf);

		IBK::IBK_Message( IBK::FormatString("  %1 [#%2]\n")
						  .arg(surf.m_displayName.toStdString())
						  .arg(surf.m_id),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}



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



void SVImportIDFDialog::on_pushButtonReplace_clicked() {
	m_returnCode = ReplaceProject;
	accept();
}


void SVImportIDFDialog::on_pushButtonMerge_clicked() {
	m_returnCode = MergeProjects;
	accept();
}


void SVImportIDFDialog::on_pushButtonImport_clicked() {
	try {
		// now transfer data to temporary VICUS project structure.
		transferData(*m_idfProject);
		QMessageBox::information(this, tr("IDF Import"),
								 tr("IDF data imported, successfully. Please review the import log for warnings!"));
	}
	catch (IBK::Exception & ex) {
		if (ex.what() == QString("Import canceled."))
			return;
		ex.writeMsgStackToError();
		QMessageBox::critical(this, tr("IDF Import"), tr("There were critical errors during the import (invalid IDF file), see log for details."));
		return;
	}

	// merge project is only active if we have a project
	m_ui->pushButtonMerge->setEnabled( SVProjectHandler::instance().isValid() );
	m_ui->pushButtonReplace->setEnabled(true);
	m_ui->pushButtonImport->setEnabled(false);
}


// *** message handler


SVImportMessageHandler::SVImportMessageHandler(QObject *parent, QPlainTextEdit *plainTextEdit) :
	QObject(parent),
	m_plainTextEdit(plainTextEdit)
{
	m_defaultMsgHandler = dynamic_cast<SVMessageHandler *>(IBK::MessageHandlerRegistry::instance().messageHandler());
	Q_ASSERT(m_defaultMsgHandler != nullptr);
	IBK::MessageHandlerRegistry::instance().setMessageHandler(this);
}


SVImportMessageHandler::~SVImportMessageHandler() {
	IBK::MessageHandlerRegistry::instance().setMessageHandler(m_defaultMsgHandler);
}


void SVImportMessageHandler::msg(const std::string& msg,
	IBK::msg_type_t t,
	const char * func_id,
	int verbose_level)
{
	if (msg.empty())
		return;
	std::string msg2;
	if (msg[msg.size()-1] == '\n')
		msg2 = msg.substr(0,msg.size()-1);
	else
		msg2 = msg;

	switch (t) {
		case IBK::MSG_WARNING :
			msg2 = "<span style=\"color:#e0c000\">" + msg2 + "</span>";
			m_plainTextEdit->appendHtml(QString::fromStdString(msg2));
		break;

		case IBK::MSG_ERROR :
			msg2 = "<span style=\"color:#d00000\">" + msg2 + "</span>";
			m_plainTextEdit->appendHtml(QString::fromStdString(msg2));
		break;

		default:
			m_plainTextEdit->appendPlainText(QString::fromStdString(msg2));
	}


	IBK::MessageHandler::msg(msg, t, func_id, verbose_level);
	if (verbose_level > m_requestedConsoleVerbosityLevel)
		return;
}


