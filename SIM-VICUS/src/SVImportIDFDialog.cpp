
/*    SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ...

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
#include <QScrollBar>

#include <SV_Conversions.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVMessageHandler.h"
#include "SVStyle.h"

#include <EP_Project.h>
#include <EP_IDFParser.h>

#include <VICUS_utilities.h>

#include <IBKMK_3DCalculations.h>

SVImportIDFDialog::SVImportIDFDialog(QWidget *parent) :
	QDialog(parent
			#ifdef Q_OS_LINUX
			, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint /*| Qt::WindowSystemMenuHint*/
			#endif
			),
	m_idfProject(new EP::Project),
	m_ui(new Ui::SVImportIDFDialog)
{
	m_ui->setupUi(this);

	//    QList<QByteArray> codecs = QTextCodec::availableCodecs();
	//    std::sort(codecs.begin(), codecs.end());
	//    for (const QByteArray & b : codecs)
	m_ui->comboBoxEncoding->addItem("UTF-8");
	m_ui->comboBoxEncoding->addItem("ISO 8859-1");
	m_ui->comboBoxEncoding->addItem("ISO 8859-15");

	m_ui->comboBoxEncoding->setCurrentText("ISO 8859-15"); // most IDF will have been created on Windows, so we use this locale by default

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

	// if successful, show dialog
	m_ui->pushButtonMerge->setEnabled(false);
	m_ui->pushButtonReplace->setEnabled(false);

	// read IDF file
	try {
		EP::IDFParser parser;
		parser.read(IBK::Path(fname.toStdString()));

		IBK::IBK_Message("Parsing IDF...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		*m_idfProject = EP::Project();
		m_idfProject->readIDF(parser);
		m_ui->pushButtonImport->setEnabled(true);

		IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		updateEncodingPreview();

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		QMessageBox::critical((QWidget*)parent(), tr("Import error"), tr("Error parsing IDF file:\n%1").arg(ex.what()));
		m_ui->pushButtonImport->setEnabled(false);
	}

	int res = exec();
	if (res == QDialog::Rejected)
		return ImportCancelled;

	return m_returnCode;
}


void updateProgress(QProgressDialog * dlg, QElapsedTimer & progressTimer, int count) {
	const unsigned int PROGRESS_TIMER_MSEC_DELAY = 400;
	double elapsed = progressTimer.elapsed();
	if (elapsed > PROGRESS_TIMER_MSEC_DELAY) {
		dlg->setValue(count);
		if (dlg->wasCanceled())
			throw IBK::Exception("Import canceled.", "[SVImportIDFDialog::transferData]");
		progressTimer.start();
	}
}



int constructionType(const std::string &conName, const EP::Project &prj, std::string &err, unsigned int &vecPos){
	int result = -1;    //not found
	/* result description
		-1 not found
		1 opaque
		2 simple glazing system
		3 simple glazing system with shade
		4 other
		9 unsupported material layers
		10 Error
	*/
	const std::vector<EP::Construction> &cons = prj.m_constructions;
	const std::vector<EP::Material> &mats = prj.m_materials;
	const std::vector<EP::WindowMaterial> &winMats = prj.m_windowMaterial;

	vecPos = 0;
	for(const EP::Construction &con : cons){
		if(con.m_name == conName){
			result = 0;
			for(const std::string &layerName : con.m_layers){

				//check for opaque layer
				bool foundOpaque = false;
				for(const EP::Material &mat : mats){

					if(mat.m_name == layerName){
						if(result > 1){
							err = "The construction '" + conName + "' is invalid. The construction has opaque and other material layers.";
							return 10;
						}
						foundOpaque = true;
						result = 1;
					}
				}

				//check for simple glazing system
				bool foundSimpleWin = false;
				for(const EP::WindowMaterial &mat : winMats){

					if(mat.m_name == layerName){
						if(result == 3 || result == 2 || result == -1){
							err = "The construction '" + conName + "' is invalid. The construction has transparent and other material layers.";
							return 10;
						}

						foundSimpleWin = true;
						result = 2;
					}
				}
				if(!(foundOpaque && foundSimpleWin)){
					err = "The construction '" + conName + "' has unsupported material layers.";
					return 9;
				}
			}
			break;
		}
		++vecPos;
	}
	return result;
}


void SVImportIDFDialog::transferData(const EP::Project & prj, unsigned int startID) {
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

	unsigned int nextID = startID;

	VICUS::Project & vp = m_importedProject; // readability improvement
	vp = VICUS::Project(); // clear any previous data

	vp.m_buildings.resize(1);
	vp.m_buildings[0].m_buildingLevels.resize(1);
	vp.m_buildings[0].m_id = ++nextID;
	vp.m_buildings[0].m_displayName = tr("Imported IDF Building Geometry");

	VICUS::BuildingLevel & bl = vp.m_buildings[0].m_buildingLevels[0];
	bl.m_id = ++nextID;
	bl.m_displayName = tr("Default building level");

	// count all elements to be imported
	int elementsToImport =
			prj.m_bsd.size()*2 +  // we loop twice here
			prj.m_constructions.size() +
			prj.m_zones.size() +
			prj.m_fsd.size()*2 +
			prj.m_shadingBuildingDetailed.size();

	QProgressDialog dlg(tr("Importing IDF project"), tr("Abort"), 0, elementsToImport, this);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.setValue(0);
	dlg.setMinimumDuration(0);

	QElapsedTimer progressTimer;
	progressTimer.start();

	int count = 0; // progress counter

	IBKMK::Vector3D minCoords(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	IBKMK::Vector3D maxCoords(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest());

	QTextCodec * codec = QTextCodec::codecForName(m_ui->comboBoxEncoding->currentText().toLocal8Bit());

	// we start with database components

	// Vector that relates IDF Material definition to VICUS Material IDs -> note that we directly modify SV Database during import
	// to avoid excessive material duplicate generation.
	// We store this as vector and not as map, because we need the index to access IDF material properties later.
	std::vector<unsigned int> idfMat2VicusMatIDs;
	std::vector<unsigned int> idfWindowGlazingSystem2VicusIDs;

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
		mat.m_id = ++nextID;
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
	IBK_ASSERT(idfMat2VicusMatIDs.size() == prj.m_materials.size());

	IBK::IBK_Message("\nImporting window glazing systems...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::WindowMaterial & m : prj.m_windowMaterial) {
		// generate VICUS::WindowGlazingSystem

		VICUS::WindowGlazingSystem glazing;
		glazing.m_id = ++nextID;
		QString matName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
		glazing.m_displayName.setEncodedString(matName.toStdString() );
		glazing.m_color = SVStyle::randomColor();
		glazing.m_modelType = VICUS::WindowGlazingSystem::MT_Simple;

		VICUS::KeywordList::setParameter(glazing.m_para, "WindowGlazingSystem::para_t", VICUS::WindowGlazingSystem::P_ThermalTransmittance, m.m_uValue);
		//create a default spline for SHGC
		std::vector<double> angles;
		std::vector<double> values;
		for (unsigned int i=0; i<10; ++i)
			angles.push_back(i*10); // in Deg

		values.push_back(1); // 0 deg
		values.push_back(1);
		values.push_back(1);
		values.push_back(1);
		values.push_back(0.98);
		values.push_back(0.94);
		values.push_back(0.86);
		values.push_back(0.69);
		values.push_back(0.37);
		values.push_back(0); // 90 deg

		// scale by nominal SHGC
		for (double & v : values)
			v *= m.m_SHGC;

		glazing.m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_name = "SHGC";
		glazing.m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_xUnit.set("Deg");
		glazing.m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_yUnit.set("---");
		glazing.m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values.setValues(angles, values);

		// color and ID don't matter for now, try to find similar material in DB
		bool found = false;
		for (const std::pair<const unsigned int, VICUS::WindowGlazingSystem> & dbMat : db.m_windowGlazingSystems) {
			if (dbMat.second.equal(&glazing) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("  %1 -> using existing window glazing system '%2' [#%3] \n")
								  .arg("'"+matName.toStdString()+"'", 40, std::ios_base::left)
								  .arg(dbMat.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbMat.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				idfWindowGlazingSystem2VicusIDs.push_back(dbMat.first);
				found = true;
				break;
			}
		}
		if (!found) {
			// no matching material found, add new material to DB
			unsigned int newID = db.m_windowGlazingSystems.add(glazing);
			IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2\n")
							  .arg("'"+matName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			idfWindowGlazingSystem2VicusIDs.push_back(newID);
		}
	}
	IBK_ASSERT(idfWindowGlazingSystem2VicusIDs.size() == prj.m_windowMaterial.size());

	std::map<std::string, std::vector<std::string>> constructionNameToFrameNames;

	// get all windows
	//search in all fsd's
	//get the frame and divider there
	for(const EP::FenestrationSurfaceDetailed &fsd : prj.m_fsd){
		bool foundFrame = false;
		//search for this window construction if frame is listed
		std::vector<std::string> &frames= constructionNameToFrameNames[fsd.m_constructionName];
		for (unsigned int i=0; i< frames.size(); ++i){
			if(frames[i] == fsd.m_frameAndDividerName){
				foundFrame = true;
				break;
			}
		}
		// not listed add frame
		if(!foundFrame){
			frames.push_back(fsd.m_frameAndDividerName);
		}
	}

	std::map<std::string, unsigned int> epFrameNameToIdFrameVicus;
	std::map<std::string, unsigned int> epFrameNameToIdDividerVicus;
	std::map<std::string, unsigned int> frameNameToFrameMaterialIds;
	IBK::IBK_Message("\nImporting frames...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Frame & frame : prj.m_frames) {
		epFrameNameToIdFrameVicus[frame.m_name] = VICUS::INVALID_ID;
		VICUS::WindowFrame frameVIC;
		QString matName = codec->toUnicode(frame.m_name.c_str()); // Mind text encoding here!

		//create a material for the frame
		VICUS::Material mat;
		VICUS::KeywordList::setParameter(frameVIC.m_para, "WindowFrame::para_t", VICUS::WindowFrame::P_Thickness, 0.1);
		bool hasFrame = false;
		if(frame.m_conductanceFrame>0){
			VICUS::KeywordList::setParameter(mat.m_para, "Material::para_t", VICUS::Material::P_Conductivity, frame.m_conductanceFrame * 0.1);
			hasFrame = true;
		}
		VICUS::KeywordList::setParameter(mat.m_para, "Material::para_t", VICUS::Material::P_Density, 500);
		VICUS::KeywordList::setParameter(mat.m_para, "Material::para_t", VICUS::Material::P_HeatCapacity, 1500);

		mat.m_displayName.setEncodedString("de:Rahmenmaterial");

		// color and ID don't matter for now, try to find similar material in DB
		bool found = false;
		if(hasFrame){
			for (const std::pair<const unsigned int, VICUS::Material> & dbMat : db.m_materials) {
				if (dbMat.second.equal(&mat) != VICUS::AbstractDBElement::Different) {
					// re-use this material
					IBK::IBK_Message( IBK::FormatString("  %1 -> using existing material '%2' [#%3] \n")
									  .arg("'"+matName.toStdString()+"'", 40, std::ios_base::left)
									  .arg(dbMat.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbMat.first),
									  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
					frameNameToFrameMaterialIds[frame.m_name] = dbMat.first;
					found = true;
					break;
				}
			}
			if (!found) {
				// no matching material found, add new material to DB
				unsigned int newID = db.m_materials.add(mat);
				IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2\n")
								  .arg("'"+matName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				frameNameToFrameMaterialIds[frame.m_name] = newID;
			}
			//add frame later
			//first add database ..
			//for(const std::pair<const unsigned int, VICUS::WindowFrame> & dbFrame : db.m)

		}
	}
	IBK_ASSERT(idfMat2VicusMatIDs.size() == prj.m_materials.size());


	// For each construction in IDF we store the respective VICUS-construction ID, and also, if the
	// referenced construction is defined in reverse.
	std::vector<std::pair<unsigned int, bool> > idfConstruction2VicusConIDs;

	struct CompareConstructions{
		int idfIdx = -1;
		int reverseConstructionIdfIdx = -1;

	};

	std::vector<std::pair<unsigned int, bool> > idfWindow2VicusWindowIDs;

	IBK::IBK_Message("\nImporting constructions...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Construction & construction : prj.m_constructions) {
		updateProgress(&dlg, progressTimer, ++count);

		// check which construction we have
		enum MaterialType{
			Opaque,
			Window,
			//            Blind,
			//            Shade,
			//            Screen,
			NUM_MT
		};

		MaterialType matType = NUM_MT;

		// generate VICUS::ConstructionType
		VICUS::Construction con;
		VICUS::Window window;
		window.m_id = ++nextID;


		QString conName = codec->toUnicode(construction.m_name.c_str()); // Mind text encoding here!
		IBK::MultiLanguageString name;
		name.setEncodedString(conName.toStdString() );
		QColor color = SVStyle::randomColor();

		try {
			// process all referenced materials and lookup matching VICUS material IDs
			for (const std::string & matLay : construction.m_layers) {
				// find material by name
				unsigned int opaqueIdx = VICUS::elementIndex(prj.m_materials, matLay);
				unsigned int windowIdx = VICUS::elementIndex(prj.m_windowMaterial, matLay);
				unsigned int blindIdx, shadeIdx, screenIdx;

				if (opaqueIdx < prj.m_materials.size()) {
					IBK_ASSERT(matType == NUM_MT || matType == Opaque);
					matType = Opaque;

					// extract size in [m]
					double thickness = prj.m_materials[opaqueIdx].m_thickness;
					con.m_materialLayers.push_back(VICUS::MaterialLayer(thickness, idfMat2VicusMatIDs[opaqueIdx]) );

				}
				else if (windowIdx < prj.m_windowMaterial.size()){
					IBK_ASSERT(matType == NUM_MT || matType == Window);
					// TODO : warn if matType already set, otherwise we have multiple layers
					matType = Window;
					window.m_idGlazingSystem = idfWindowGlazingSystem2VicusIDs[windowIdx];
				}
				///TODO Dirk: add shading when available

				if (matType == NUM_MT) {
					// also convert names in error message
					throw IBK::Exception(IBK::FormatString("Material '%1' referenced from construction '%2' is not defined in IDF file.")
										 .arg(codec->toUnicode(matLay.c_str()).toStdString()).arg(conName.toStdString()), FUNC_ID);
				}
			}

		} catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			idfConstruction2VicusConIDs.push_back( std::make_pair(VICUS::INVALID_ID, false) ); // insert dummy ID
			idfWindow2VicusWindowIDs.push_back( std::make_pair(VICUS::INVALID_ID, false) ); // insert dummy ID
			continue; // skip construction
		}
		//now we have parsed all layers of current construction
		idfConstruction2VicusConIDs.push_back(std::make_pair(VICUS::INVALID_ID,false));
		idfWindow2VicusWindowIDs.push_back(std::make_pair(VICUS::INVALID_ID,false));
		switch(matType){
		case Opaque:{
			con.m_displayName = name;
			con.m_color = color;
			con.m_dataSource.setEncodedString("IDF Import");

			VICUS::Construction conRev(con);
			conRev.m_id = ++nextID;
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
					idfConstruction2VicusConIDs.back() = std::make_pair(dbCon.first, false) ; // not a reverse construction
					found = true;
					break;
				}
				if (dbCon.second.equal(&conRev) != VICUS::AbstractDBElement::Different) {
					// re-use this construction
					IBK::IBK_Message( IBK::FormatString("  %1 -> using existing construction '%2' [#%3] (reversed)\n")
									  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left)
									  .arg(dbCon.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbCon.first),
									  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
					idfConstruction2VicusConIDs.back() = std::make_pair(dbCon.first, true) ; // a reverse construction
					found = true;
					break;
				}
			}

			if (!found) {
				// no matching construction found, add new to DB
				unsigned int newID = db.m_constructions.add(con);
				IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2\n")
								  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				idfConstruction2VicusConIDs.back() = std::make_pair(newID, false) ; // not a reverse construction
			}
		}
			break;
		case Window:{
			window.m_displayName = name;
			window.m_color = color;
			window.m_dataSource = "IDF Import";
			for(unsigned int i=0; i<constructionNameToFrameNames[construction.m_name].size(); ++i){
				const std::string &frameName = constructionNameToFrameNames[construction.m_name][i];
				if(frameName.empty()){
					window.m_methodFrame = VICUS::Window::M_None;
					window.m_methodDivider = VICUS::Window::M_None;
				}
				else{
					VICUS::WindowFrame winFrame;
					VICUS::WindowDivider windDivider;


				}
			}
			//now we have no frame
			///TODO Dirk->Andreas wie können wir die Frames hinzufügen die aber erst in Verbindung mit den FSD's bereit stehen?

			bool found = false;
			// now check if window construction (or its reverse) does already exist in VICUS DB
			// color and ID don't matter for now, try to find similar material in DB
			for (const std::pair<const unsigned int, VICUS::Window> & dbWinCon : db.m_windows) {
				if (dbWinCon.second.equal(&window) != VICUS::AbstractDBElement::Different) {
					// re-use this construction
					IBK::IBK_Message( IBK::FormatString("  %1 -> using existing window construction '%2' [#%3]\n")
									  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left)
									  .arg(dbWinCon.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbWinCon.first),
									  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
					idfWindow2VicusWindowIDs.back() = std::make_pair(dbWinCon.first, false) ; // not a reverse construction
					found = true;
					break;
				}
			}

			if (!found) {
				// no matching window construction found, add new to DB
				unsigned int newID = db.m_windows.add(window);
				IBK::IBK_Message( IBK::FormatString("  %1 -> imported with ID #%2\n")
								  .arg("'"+conName.toStdString()+"'", 40, std::ios_base::left).arg(newID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				idfWindow2VicusWindowIDs.back() = std::make_pair(newID, false) ; // not a reverse construction
			}
		}
			break;
		case NUM_MT:
			// only for compiler
			break;

		}
	}
	IBK_ASSERT(idfConstruction2VicusConIDs.size() == prj.m_constructions.size() && idfWindow2VicusWindowIDs.size() == prj.m_constructions.size() );


	// *** Inside boundary conditions ("surface") ***

	unsigned int bcIDSurface = VICUS::INVALID_ID; // holds the ID of inside surfaces
	{
		VICUS::BoundaryCondition bcSurface;
		bcSurface.m_id = ++nextID;
		QString bcName = tr("Inside surface");
		bcSurface.m_displayName.setEncodedString(bcName.toStdString());
		bcSurface.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::MT_Constant;
		VICUS::KeywordList::setParameter(bcSurface.m_heatConduction.m_para, "InterfaceHeatConduction::para_t",
										 VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 6.8);
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
		bc.m_id = ++nextID;
		QString bcName = tr("Ground surface");
		bc.m_displayName.setEncodedString(bcName.toStdString());
		bc.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::MT_Constant;
		bc.m_heatConduction.m_otherZoneType = VICUS::InterfaceHeatConduction::OZ_Constant;
		VICUS::KeywordList::setParameter(bc.m_heatConduction.m_para, "InterfaceHeatConduction::para_t",
										 VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 1000);
		// TODO : how to set the remaining parameters?


		bool found = false;
		for (const std::pair<const unsigned int, VICUS::BoundaryCondition> & dbBC : db.m_boundaryConditions) {
			if (dbBC.second.equal(&bc) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("Using existing boundary condition '%1' [#%2]\n")
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
			IBK::IBK_Message( IBK::FormatString("Added new boundary condition '%1' with ID #%2\n")
							  .arg(bcName.toStdString()).arg(bcIDSurface), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}


	// *** Adiabatic boundary conditions ("adiabatic") ***

	unsigned int bcIDAdiabatic= VICUS::INVALID_ID;
	{
		VICUS::BoundaryCondition bc;
		bc.m_id = ++nextID;
		QString bcName = tr("Adiabatic surface");
		bc.m_displayName.setEncodedString(bcName.toStdString());
		bc.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::MT_Constant;
		VICUS::KeywordList::setParameter(bc.m_heatConduction.m_para, "InterfaceHeatConduction::para_t", VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 0);
		// TODO : how to set the remaining parameters?

		bool found = false;
		for (const std::pair<const unsigned int, VICUS::BoundaryCondition> & dbBC : db.m_boundaryConditions) {
			if (dbBC.second.equal(&bc) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("Using existing boundary condition '%1' [#%2]\n")
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
			IBK::IBK_Message( IBK::FormatString("Added new boundary condition '%1' with ID #%2\n")
							  .arg(bcName.toStdString()).arg(bcIDSurface), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}


	// *** Outside boundary conditions ("outdoors") ***

	unsigned int bcIDOutside= VICUS::INVALID_ID;
	{
		VICUS::BoundaryCondition bc;
		bc.m_id = ++nextID;
		QString bcName = tr("Outside surface");
		bc.m_displayName.setEncodedString(bcName.toStdString());
		bc.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::MT_Constant;
		VICUS::KeywordList::setParameter(bc.m_heatConduction.m_para, "InterfaceHeatConduction::para_t",
										 VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 17);
		// TODO : how to set the remaining parameters?

		bool found = false;
		for (const std::pair<const unsigned int, VICUS::BoundaryCondition> & dbBC : db.m_boundaryConditions) {
			if (dbBC.second.equal(&bc) != VICUS::AbstractDBElement::Different) {
				// re-use this material
				IBK::IBK_Message( IBK::FormatString("Using existing boundary condition '%1' [#%2]\n")
								  .arg(dbBC.second.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(dbBC.first),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				bcIDOutside = dbBC.first;
				found = true;
				break;
			}
		}
		if (!found) {
			// no matching BC found, add new BC to DB
			bc.m_color = SVStyle::randomColor();
			bcIDOutside = db.m_boundaryConditions.add(bc);
			IBK::IBK_Message( IBK::FormatString("Added new boundary condition '%1' with ID #%2\n")
							  .arg(bcName.toStdString()).arg(bcIDSurface), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}

	// *** Zone ***

	// a map that relates zone name to index in the VICUS room vector
	std::map<std::string, unsigned int>    mapZoneNameToIdx;
	IBK::IBK_Message("\nImporting zones...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::Zone & z : prj.m_zones) {
		updateProgress(&dlg, progressTimer, ++count);

		VICUS::Room r;
		r.m_id = ++nextID;
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
	std::set<std::string> brokenBSD;

	//fsd name EP, surface id VICUS, Idx subSurface VICUS
	std::map<std::string, std::pair<unsigned int, unsigned int>> mapFsdNameIDmap;

	// import all building surface detailed -> opaque surfaces
	IBK::IBK_Message("\nImporting surfaces...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::BuildingSurfaceDetailed &bsd : prj.m_bsd) {
		updateProgress(&dlg, progressTimer, ++count);

		const auto & zoneIt = mapZoneNameToIdx.find(bsd.m_zoneName);
		if (zoneIt == mapZoneNameToIdx.end())
			throw IBK::Exception(IBK::FormatString("Zone name '%1' does not exist, which is referenced in "
												   "BuildingSurface:Detailed '%2'").arg(bsd.m_zoneName)
								 .arg(bsd.m_name), FUNC_ID);
		unsigned idx = zoneIt->second;

		VICUS::Surface surf;
		surf.m_id = ++nextID;
		surf.m_displayName = codec->toUnicode(bsd.m_name.c_str()); // Mind text encoding here!

		// set the polygon of the BSD in the surface; the polygon will be checked and the triangulation will be computed,
		// however, yet without holes
		VICUS::Polygon3D p(bsd.m_polyline);
		// TODO : Error handling?
		surf.setPolygon3D( p );

		// we can only import a subsurface, if the surface itself has a valid polygon
		if (!surf.geometry().isValid() && surf.polygon3D().vertexes().size() > 2) {
//			surf.setPolygon3D( VICUS::Polygon3D( bsd.m_polyline ) ); // for debugging purposes - to see, why polygon isn't valid
			const std::vector<IBKMK::Vector3D> &poly3D = bsd.m_polyline;

			IBK::IBK_Message(IBK::FormatString("  %3.%1 [#%2]\n : Geometry of imported surface is broken. Trying to heal it!")
							 .arg(surf.m_displayName.toStdString())
							 .arg(surf.m_id)
							 .arg(bl.m_rooms[idx].m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);

			// TODO  Stephan, there can be a number of reasons why the polygon is broken: collinear points, only 2 points,
			//                winding... we should check also for these problems before trying to fix the rest. Otherwise
			//                we might end up using not initialized normal vectors etc.

			// We assume that the polyon is invalid due to rounding errors that cause one or more vertexes to be slightly
			// out of the plane. The plane is defined by the points 0 and 1 and the polygon's normal vector.
			// We now correct all vertexes that are too far away from the plane by replacing their coordinates with
			// the coordinates of the orthogonal projection point onto the surface's plane.
			//
			// However, it may just be, that the offending vertex is vertex #0 or #1, hence the plane itself (and the
			// normal vector constructed for it) are botched. So, we use the "democratic" fix and select those
			// two points of the polygon that yield a plane where the majority of the vertexes have the least deviation.
			// We do this by checking all plane variants that could possibly be created from the polygon.

#ifdef POLYGON2D
			surf.healGeometry(poly3D);
#endif
			// we can only import a subsurface, if the surface itself has a valid polygon
			if ( surf.geometry().polygon3D().vertexes().size() != bsd.m_polyline.size() ||  !surf.geometry().isValid()) {
				IBK::IBK_Message(IBK::FormatString("  %3.%1 [#%2] : Geometry of Surface is still broken. Import skipped!\n")
								 .arg(surf.m_displayName.toStdString())
								 .arg(surf.m_id)
								 .arg(bl.m_rooms[idx].m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
				brokenBSD.insert(bsd.m_name);
				continue;
			} else {
				IBK::IBK_Message(IBK::FormatString("  %3.%1 [#%2] : Geometry healed and reconstructing points with their projection onto the plane.\n")
								 .arg(surf.m_displayName.toStdString())
								 .arg(surf.m_id)
								 .arg(bl.m_rooms[idx].m_displayName.toStdString()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD );
			}
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
		updateProgress(&dlg, progressTimer, ++count);

		QString bsdName = codec->toUnicode(bsd.m_name.c_str()); // Mind text encoding here!

		// *** Components ***

		// we first create a component with the referenced construction
		VICUS::Component com;
		com.m_id = ++nextID;
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
		// protect against invalid reference of transparent construction where an opaque construction is needed
		if (com.m_idConstruction == VICUS::INVALID_ID)
			throw IBK::Exception(IBK::FormatString("Construction '%1' referenced from BSD '%2' is not an opaque construction.")
								 .arg(codec->toUnicode(bsd.m_constructionName.c_str()).toStdString())
								 .arg(bsdName.toStdString()), FUNC_ID);

		const VICUS::Construction * con = db.m_constructions[com.m_idConstruction];
		IBK_ASSERT(con != nullptr);
		com.m_displayName.setEncodedString(QtExt::MultiLangString2QString(con->m_displayName).toStdString() + "-component");
		bool inverted = idfConstruction2VicusConIDs[conIdx].second; // if true, this is the inverted variant of a construction

		// now create boundary conditions
		unsigned int otherSurfaceID = VICUS::INVALID_ID;

		switch (bsd.m_outsideBoundaryCondition) {

		// OC_Surface represents a connection from one wall surface to the other
		case EP::BuildingSurfaceDetailed::OC_Surface : {
			// we require an opposite surface set
			if (bsd.m_outsideBoundaryConditionObject.empty())
				throw IBK::Exception(IBK::FormatString("BSD '%1' of type 'Surface' does not reference outside BC object.")
									 .arg(bsdName.toStdString()), FUNC_ID);
			com.m_idSideABoundaryCondition = bcIDSurface;
			com.m_idSideBBoundaryCondition = bcIDSurface;

			// store ID of other surface
			if ( brokenBSD.find(bsd.m_outsideBoundaryConditionObject) != brokenBSD.end() )
				continue;

			otherSurfaceID = mapBsdNameIDmap[bsd.m_outsideBoundaryConditionObject];
			IBK_ASSERT(otherSurfaceID != 0);
		} break;

		case EP::BuildingSurfaceDetailed::OC_Ground : {
			com.m_idSideABoundaryCondition = bcIDSurface;
			com.m_idSideBBoundaryCondition = bcIDGround;
		} break;

		case EP::BuildingSurfaceDetailed::OC_Outdoors : {
			com.m_idSideABoundaryCondition = bcIDSurface;
			com.m_idSideBBoundaryCondition = bcIDOutside;
		} break;

		case EP::BuildingSurfaceDetailed::OC_Adiabatic : {
			com.m_idSideABoundaryCondition = bcIDSurface;
			//no other boundary condition --> adiabatic
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

		if ( surfID == 0 ) {
			// check if bsd is broken
			if (brokenBSD.find(bsd.m_name) != brokenBSD.end() ) {
				IBK::IBK_Message( IBK::FormatString("  skipped '%1' [#%2] (has been broken)\n")
								  .arg(codec->toUnicode(bsd.m_name.c_str()).toStdString()).arg(surfID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

				continue;
			}
		}

		IBK_ASSERT(surfID != 0);
		// skip interface generation, if surface has been handled already
		if (connectedSurfaces.find(surfID) != connectedSurfaces.end()) {
			IBK::IBK_Message( IBK::FormatString("  skipped '%1' [#%2] (has been connected already)\n")
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
	vp.updatePointers(); // remember to update pointer reference table before calling surfaceByID

	IBK::IBK_Message("\nImporting windows (sub-surfaces)...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::FenestrationSurfaceDetailed &fsd : prj.m_fsd) {
		updateProgress(&dlg, progressTimer, ++count);

		QString fsdName = codec->toUnicode(fsd.m_name.c_str()); // Mind text encoding here!
		QString bsdName = codec->toUnicode(fsd.m_bsdName.c_str()); // Mind text encoding here!

		// look up surface that this fenestration belongs to
		const auto & bsd = mapBsdNameIDmap.find(fsd.m_bsdName);
		if (bsd == mapBsdNameIDmap.end()) {
			IBK::IBK_Message(IBK::FormatString("BuildingSurface:Detailed name '%1' does not exist, which is "
											   "referenced in FenestrationSurfaceDetailed '%2', skipped.")
							 .arg(bsdName.toStdString()).arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
			continue;
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
		subSurf.m_id = ++nextID;
		subSurf.m_displayName = codec->toUnicode(fsd.m_name.c_str()); // Mind text encoding here!
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


		// remember bsd/surface name - id association
		mapFsdNameIDmap[fsd.m_name] = std::pair<unsigned int, unsigned int> (surf->m_id, subs.size()-1);
	}

#if 1
	IBK::IBK_Message("\nImporting window components...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::FenestrationSurfaceDetailed &fsd : prj.m_fsd) {
		updateProgress(&dlg, progressTimer, ++count);

		QString fsdName = codec->toUnicode(fsd.m_name.c_str()); // Mind text encoding here!
		QString bsdName = codec->toUnicode(fsd.m_bsdName.c_str()); // Mind text encoding here!
		// look up surface that this fenestration belongs to
		const auto & bsd = mapBsdNameIDmap.find(fsd.m_bsdName);
		if (bsd == mapBsdNameIDmap.end()) {
			IBK::IBK_Message(IBK::FormatString("BuildingSurface:Detailed name '%1' does not exist, which is "
											   "referenced in FenestrationSurfaceDetailed '%2', skipped")
							 .arg(bsdName.toStdString()).arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
			continue;
		}

		// *** Window Components ***

		// we first create a component with the referenced construction
		VICUS::SubSurfaceComponent com;
		com.m_id = ++nextID;

		//set up type
		switch (fsd.m_surfaceType ) {
			case EP::FenestrationSurfaceDetailed::ST_Door:
				com.m_type = VICUS::SubSurfaceComponent::CT_Door;
				break;
			case EP::FenestrationSurfaceDetailed::ST_GlassDoor:
			case EP::FenestrationSurfaceDetailed::ST_Window:
				com.m_type = VICUS::SubSurfaceComponent::CT_Window;
				break;
			case EP::FenestrationSurfaceDetailed::NUM_ST: {
				IBK::IBK_Message(IBK::FormatString("FSD '%1' does not have a supported type.")
								 .arg(codec->toUnicode(fsd.m_constructionName.c_str()).toStdString())
								 .arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
				continue;
			}
		}

		// lookup construction
		unsigned int conIdx = VICUS::elementIndex(prj.m_constructions, fsd.m_constructionName);
		if (conIdx == prj.m_constructions.size()) {
			// also convert names in error message
			IBK::IBK_Message(IBK::FormatString("Construction '%1' referenced from FSD '%2' is not defined in IDF file.")
							 .arg(codec->toUnicode(fsd.m_constructionName.c_str()).toStdString())
							 .arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
			continue;
		}

		// lookup construnction/window ID
		unsigned int idWindow = idfWindow2VicusWindowIDs[conIdx].first;
		unsigned int idConstruction = idfConstruction2VicusConIDs[conIdx].first;

		// we need to distinguish between windows and doors -> windows are stored in transparent construction ID vector
		if (com.m_type == VICUS::SubSurfaceComponent::CT_Window) {
			// if this index is INVALID_ID, we have referenced an opaque construction, but we need a window construction
			com.m_idWindow = idWindow;
			if (idWindow == VICUS::INVALID_ID) {
				if (idConstruction == VICUS::INVALID_ID)
					IBK::IBK_Message(IBK::FormatString("Construction '%1' referenced from FSD '%2' was not imported (due to errors, see log above).")
									 .arg(codec->toUnicode(fsd.m_constructionName.c_str()).toStdString())
									 .arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
				else
					IBK::IBK_Message(IBK::FormatString("Construction '%1' referenced from FSD '%2' is not a transparent construction, skipped.")
									 .arg(codec->toUnicode(fsd.m_constructionName.c_str()).toStdString())
									 .arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);

				// we keep the subsurface component and generate the component instance, but with a dummy window
				if (!db.m_windows.empty())
					com.m_idWindow = db.m_windows.begin()->first;
				else
					continue;
			}

			const VICUS::Window * window = db.m_windows[com.m_idWindow];
			IBK_ASSERT(window != nullptr);
			com.m_displayName.setEncodedString(QtExt::MultiLangString2QString(window->m_displayName).toStdString() + "-component");

			// frame and divider
			if (!fsd.m_frameAndDividerName.empty()) {

			}
		}
		else {
			// lookup matching VICUS::Construction ID
			com.m_idConstruction = idConstruction;
			// if this index is INVALID_ID, we have referenced a transparent construction, but we need a window construction
			if (idConstruction == VICUS::INVALID_ID) {
				if (idWindow == VICUS::INVALID_ID)
					IBK::IBK_Message(IBK::FormatString("Construction '%1' referenced from FSD '%2' was not imported (due to errors, see log above).")
									 .arg(codec->toUnicode(fsd.m_constructionName.c_str()).toStdString())
									 .arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
				else
					IBK::IBK_Message(IBK::FormatString("Construction '%1' referenced from FSD '%2' is not not an opaque construction, skipped.")
									 .arg(codec->toUnicode(fsd.m_constructionName.c_str()).toStdString())
									 .arg(fsdName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
				com.m_idConstruction = db.m_constructions.begin()->first;
			}

			const VICUS::Construction * con = db.m_constructions[com.m_idConstruction];
			IBK_ASSERT(con != nullptr);
			com.m_displayName.setEncodedString(QtExt::MultiLangString2QString(con->m_displayName).toStdString() + "-component");
		}

		// now create boundary conditions
		//not used now
		//unsigned int otherSurfaceID = VICUS::INVALID_ID;

		// TODO Dirk, reverse construction bei Door


		// For now, we only have outside surfaces, i.e. FSD without outside boundary condition object
		if(fsd.m_outsideBoundaryConditionObject.empty()){
			//now we assume we have an outside window
			com.m_idSideABoundaryCondition = bcIDSurface;
			com.m_idSideBBoundaryCondition = bcIDOutside;
		}
		else{
			//now we assume we have an internal window
			///TODO Dirk->Andreas: unterstützen wir derzeit schon SubSurfaces bei Innenwänden?
			continue;    // this is not supported yet
		}

		unsigned int comId = VICUS::INVALID_ID;
		// check, if we have such a component already
		bool found = false;
		for (const std::pair<const unsigned int, VICUS::SubSurfaceComponent> & dbElement : db.m_subSurfaceComponents) {
			if (dbElement.second.equal(&com) != VICUS::AbstractDBElement::Different) {
				// re-use this sub surface component
				IBK::IBK_Message( IBK::FormatString("  -> using existing sub surface component '%1' [#%2]\n")
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
			comId = db.m_subSurfaceComponents.add(com);
			IBK::IBK_Message( IBK::FormatString("  -> added new sub surface component '%1' [#%2]\n")
							  .arg(com.m_displayName.string(IBK::MultiLanguageString::m_language, true)).arg(comId), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}

		// we skip the next block because we only support outside windows for now
		// check that the current surface hasn't been connected, yet
		//        unsigned int surfID = mapBsdNameIDmap[fsd.m_name];
		//        IBK_ASSERT(surfID != 0);
		//        // skip interface generation, if surface has been handled already
		//        if (connectedSurfaces.find(surfID) != connectedSurfaces.end()) {
		//            IBK::IBK_Message( IBK::FormatString("  skipped '%1' [#%2] (has been connected already)\n")
		// .arg(codec->toUnicode(fsd.m_name.c_str()).toStdString()).arg(surfID), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		//            continue;
		//        }

		//        connectedSurfaces.insert(surfID);
		//        if (otherSurfaceID != VICUS::INVALID_ID)
		//            connectedSurfaces.insert(otherSurfaceID);

		// now create a new component instance
		VICUS::SubSurfaceComponentInstance ssci;
		ssci.m_id = VICUS::uniqueId(vp.m_subSurfaceComponentInstances);

		if(mapFsdNameIDmap.find(fsd.m_name) == mapFsdNameIDmap.end()){
			IBK::IBK_Message(IBK::FormatString("FSD %1 is not contained in mapping.").arg(fsd.m_name), IBK::MSG_WARNING,
							 FUNC_ID, IBK::VL_STANDARD);
			continue;
		}
		VICUS::Surface *surf = vp.surfaceByID(mapFsdNameIDmap[fsd.m_name].first);

		IBK_ASSERT(surf != nullptr);

		ssci.m_idSideASurface = surf->subSurfaces()[mapFsdNameIDmap[fsd.m_name].second].m_id;
		ssci.m_idSideBSurface = VICUS::INVALID_ID;

		ssci.m_idSubSurfaceComponent = comId;

		VICUS::SubSurface * subsurf = vp.subSurfaceByID(ssci.m_idSideASurface);
		// Mind: nullptr is possible here!
		if (subsurf != nullptr) {
			switch (com.m_type) {
				case VICUS::SubSurfaceComponent::CT_Window:
					subsurf->m_color = QColor(96,96,255,64);
					break;
				case VICUS::SubSurfaceComponent::CT_Door:
				case VICUS::SubSurfaceComponent::CT_Miscellaneous:
				case VICUS::SubSurfaceComponent::NUM_CT:
					subsurf->m_color = QColor(164,164,164,255);
			}
		}

		//not used now
		/*
		if (otherSurfaceID != VICUS::INVALID_ID)
			IBK::IBK_Message( IBK::FormatString("    '%1' [#%2] connected to surface '%3' [#%4]\n")
.arg(codec->toUnicode(fsd.m_name.c_str()).toStdString()).arg(surfID)
.arg(codec->toUnicode(fsd.m_outsideBoundaryConditionObject.c_str()).toStdString()).arg(otherSurfaceID),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		else {
			// TODO : distinguish between ambient and ground
			IBK::IBK_Message( IBK::FormatString("    '%1' [#%2] connected to outside\n")
.arg(codec->toUnicode(fsd.m_name.c_str()).toStdString()).arg(surfID),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
		*/
		vp.m_subSurfaceComponentInstances.push_back(ssci);
	}
#endif

	// *** ShadingBuildingDetailed ***

	IBK::IBK_Message("\nShading geometry (ShadingBuildingDetailed)...\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	for (const EP::ShadingBuildingDetailed &sh : prj.m_shadingBuildingDetailed) {
		updateProgress(&dlg, progressTimer, ++count);

		VICUS::Surface surf;
		surf.m_id = ++nextID;
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
	vp.m_viewSettings.m_farDistance = 10000;
	vp.m_viewSettings.m_gridWidth = maxDist*2;
	if (maxDist < 10) {
		vp.m_viewSettings.m_gridSpacing = 0.1;
		vp.m_viewSettings.m_gridWidth = 20;
	}
	else if (maxDist < 100) {
		vp.m_viewSettings.m_gridSpacing = 1;
		vp.m_viewSettings.m_gridWidth = 200;
	}
	else if (maxDist < 500) {
		vp.m_viewSettings.m_gridSpacing = 10;
		vp.m_viewSettings.m_gridWidth = 1000;
	}
	else if (maxDist < 1000) {
		vp.m_viewSettings.m_gridSpacing = 50;
		vp.m_viewSettings.m_gridWidth = 2000;
	}
	else
		vp.m_viewSettings.m_gridSpacing = 100;

	// now that we have the center of the building, we can compute the horizontal transformation
	double centerX = 0.5*(maxCoords.m_x + minCoords.m_x);
	double centerY = 0.5*(maxCoords.m_y + minCoords.m_y);

	// now translate entire geometry to the center of the scene
	IBKMK::Vector3D trans(-centerX, -centerY, 0);
	for (VICUS::Surface & s : vp.m_plainGeometry) {
		std::vector<IBKMK::Vector3D> vertexes = s.geometry().polygon3D().vertexes();
		for (IBKMK::Vector3D & v : vertexes)
			v += trans;
		s.setPolygon3D(VICUS::Polygon3D(vertexes));
	}

	for (VICUS::Building & b : vp.m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces) {
					std::vector<IBKMK::Vector3D> vertexes = s.geometry().polygon3D().vertexes();
					for (IBKMK::Vector3D & v : vertexes)
						v += trans;
					s.setPolygon3D(VICUS::Polygon3D(vertexes));
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


void SVImportIDFDialog::on_pushButtonImport_clicked() {
	try {
		// we assume, that the user want's to add the project to the existing project, so
		// start numbering in higher ID space
		unsigned int nextID = 10000;
		if (SVProjectHandler::instance().isValid())
			nextID = project().nextUnusedID();
		// now transfer data to temporary VICUS project structure.
		transferData(*m_idfProject, nextID);
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
	m_plainTextEdit->clear();
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
		msg2 = "<span style=\"color:#e0c000\">" + msg2 + "</span><br>";
		m_plainTextEdit->appendHtml(QString::fromStdString(msg2));
		break;

	case IBK::MSG_ERROR :
		msg2 = "<span style=\"color:#d00000\">" + msg2 + "</span><br>";
		m_plainTextEdit->appendHtml(QString::fromStdString(msg2));
		break;

	default:
		m_plainTextEdit->appendPlainText(QString::fromStdString(msg2));
	}
	m_plainTextEdit->verticalScrollBar()->setValue(m_plainTextEdit->verticalScrollBar()->maximum());

	m_defaultMsgHandler->msg(msg, t, func_id, verbose_level);
	if (verbose_level > m_requestedConsoleVerbosityLevel)
		return;
}



void SVImportIDFDialog::on_comboBoxEncoding_currentIndexChanged(int) {
	m_ui->plainTextEdit->clear();
	updateEncodingPreview();
}

void SVImportIDFDialog::updateEncodingPreview() {
	// display zone and construction names in selected encoding
	QTextCodec * codec = QTextCodec::codecForName(m_ui->comboBoxEncoding->currentText().toLocal8Bit());
	IBK::IBK_Message(IBK::FormatString("Preview names with non-latin1 characters in selected encoding '%1'\n").arg(m_ui->comboBoxEncoding->currentText().toStdString()));
	IBK::IBK_Message("(only names with non-latin 1 characters are shown...if there aren't any, the encoding doesn't matter)\n");
	if (!m_idfProject->m_materials.empty()) {
		IBK::IBK_Message("\nMaterials:\n");
		IBK::MessageIndentor indent;(void)indent;
		for (const EP::Material & m : m_idfProject->m_materials) {
			bool haveNonLatin1 = false;
			for (char ch : m.m_name)
				if ((unsigned int)ch > 128) {
					haveNonLatin1 = true;
					break;
				}
			if (!haveNonLatin1)
				continue;
			QString matName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
			IBK::IBK_Message(matName.toStdString() + "\n", IBK::MSG_PROGRESS);
		}
	}
	if (!m_idfProject->m_constructions.empty()) {
		IBK::IBK_Message("\nConstructions:\n");
		IBK::MessageIndentor indent;(void)indent;
		for (const EP::Construction & m : m_idfProject->m_constructions) {
			bool haveNonLatin1 = false;
			for (char ch : m.m_name)
				if ((unsigned int)ch > 128) {
					haveNonLatin1 = true;
					break;
				}
			if (!haveNonLatin1)
				continue;
			QString matName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
			IBK::IBK_Message(matName.toStdString() + "\n", IBK::MSG_PROGRESS);
		}
	}
	if (!m_idfProject->m_zones.empty()) {
		IBK::IBK_Message("\nZones:\n");
		IBK::MessageIndentor indent;(void)indent;
		for (const EP::Zone & m : m_idfProject->m_zones) {
			bool haveNonLatin1 = false;
			for (char ch : m.m_name)
				if ((unsigned int)ch > 128) {
					haveNonLatin1 = true;
					break;
				}
			if (!haveNonLatin1)
				continue;
			QString matName = codec->toUnicode(m.m_name.c_str()); // Mind text encoding here!
			IBK::IBK_Message(matName.toStdString() + "\n", IBK::MSG_PROGRESS);
		}
	}

}

