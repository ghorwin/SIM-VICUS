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

#include "SVLCA.h"


#include <VICUS_EpdCategorySet.h>
#include <VICUS_ComponentInstance.h>

#include <IBK_FileReader.h>
#include <IBK_StringUtils.h>

namespace SV {

//check if element exists with ID
template <typename T>
T elementExists(std::map<unsigned int, T> database, unsigned int id, std::string displayName,
				std::string idName, std::string objectType){
	FUNCID(LCA::elementExists);
	if(database.find(id) == database.end())
		IBK::Exception(IBK::FormatString("%3 id %1 of %4 '%2' is not contained in database. ")
					   .arg(id).arg(displayName).arg(idName).arg(objectType), FUNC_ID);
	return  database[id];
}


/*! gets all needed parameter values for the epd calculation and adds them to the related component. */
void addEpdMaterialToComponent(VICUS::EpdDataset epd, LCA::LCAComponentResult &comp, LCA::LCAComponentResult &comp2,
					double lifeCycle, double thickness, double rho, int idx = 0, double adjustment = 1.2){
#if 0
	FUNCID(LCA::addEpdMaterialToComponent);

	for(unsigned int i=0; i<VICUS::EpdDataset::NUM_P; ++i){
		//skip empty values
		if(epd.m_para[i].value == 0)
			continue;

		IBK::Unit unit = epd.m_para[i].unit();
		//mass material
		double mass = rho * comp.m_area  * thickness;

		//take specific epd value for this category
		double val = epd.m_para[i].get_value(unit);
		QString &refUnit = epd.m_referenceUnit;
		val *= mass;
		if(QString::compare("kg", refUnit, Qt::CaseInsensitive) == 0){
			//do nothing
		}
		else if (QString::compare("m3", refUnit, Qt::CaseInsensitive) == 0) {
			//referencequantity in [m3]
			if(epd.m_para[VICUS::EpdDataset::P_DryDensity].get_value("kg/m3") > 0)
				val /= epd.m_para[VICUS::EpdDataset::P_DryDensity].get_value("kg/m3");
		}
		else if (QString::compare("m2", refUnit, Qt::CaseInsensitive) == 0) {
			//referencequantity in [m2]
			if(epd.m_para[VICUS::EpdDataset::P_AreaDensity].get_value("kg/m2") > 0)
			val /= epd.m_para[VICUS::EpdDataset::P_AreaDensity].get_value("kg/m2");
		}
		else
			throw IBK::Exception(IBK::FormatString("No valid specific Unit of epd material '%1' available.").
								 arg(epd.m_displayName.string()), FUNC_ID);
		val /= epd.m_referenceQuantity;

		val *= adjustment;

		//Das könnten wir auch später machen - wir überlegen hier nochmal
		/// TODO Mira
		//val /= netFloorArea;
		//val /= 50;					//reporting period [a]

		comp.addValue(idx, static_cast<VICUS::EpdDataset::para_t>(i),
					  IBK::Parameter(epd.m_para[i].name, val, unit));
		if(lifeCycle >= 0)
			comp2.addValue(idx, static_cast<VICUS::EpdDataset::para_t>(i),
						   IBK::Parameter(epd.m_para[i].name, val*lifeCycle, unit));
	}
#endif
}



void LCA::calculateLCA()
{
#if 0
	FUNCID(LCA::calculateLCA);

	/*! Summarize all components with all constructions and material layers.
		Categorize all construction with their surface areas.
	*/

	/* Annahmen: diese Strukturen müssen umbenannt werden. */
	std::map<unsigned int, VICUS::Component>				m_dbComponents;
	std::map<unsigned int, VICUS::Construction>				m_dbConstructions;
	std::map<unsigned int, VICUS::Material>					m_dbOpaqueMaterials;
	std::map<unsigned int, VICUS::EPDDataset>				m_dbEPDs;


	struct MatEpd{
		VICUS::EPDDataset m_epdA;
		VICUS::EPDDataset m_epdB;
		VICUS::EPDDataset m_epdC;
		VICUS::EPDDataset m_epdD;
	};

	std::map<unsigned int, LCAComponentResult>		compRes;
	std::map<unsigned int, LCAComponentResult>		compResErsatz;

	//holds the data for each material
	std::map<unsigned int, MatEpd>					materialIdAndEpd;
	double netFloorArea = m_building.m_netFloorArea;

	/* Calculate all surface areas according to all components. */
	for (auto &bl : m_building.m_buildingLevels) {
		for (auto &r : bl.m_rooms) {
			for (auto &s : r.m_surfaces) {
				const VICUS::Surface &surf = s;

				// get component
				const VICUS::ComponentInstance * compInstance = s.m_componentInstance;
				if (compInstance != nullptr) {
					VICUS::Component comp = elementExists<VICUS::Component>(m_dbComponents, compInstance->m_idComponent,
													s.m_displayName.toStdString(),"Component", "surface");
					//save surface area
					compRes[comp.m_id].m_area += surf.geometry().area();
				}
				else {
					/// TODO : error handling if component instance pointer is empty (no component associated)
				}
			}
		}
	}

	//calculate all lca for each component
	for (auto &c : compRes) {
		const VICUS::Component &comp = m_dbComponents[c.first];

		//opaque construction
		if(comp.m_idConstruction != VICUS::INVALID_ID){
			//get construction
			///TODO Dirk baufähig gemacht müsste rückgängig gemacht werden
			VICUS::Construction constr;
//					elementExists<VICUS::Construction>(m_dbConstructions, comp.m_idOpaqueConstruction,
//													   comp.m_displayName.toStdString(),"Construction",
//													   "component");

			//calculate each construction
			for(auto l : constr.m_materialLayers){
				//check if material exists
				VICUS::Material mat =
						elementExists<VICUS::Material>(m_dbOpaqueMaterials, l.m_idMaterial,
														   constr.m_displayName.string(),
														   "Material",
														   "construction");

				//material exists already in the new user database
				if(materialIdAndEpd.find(mat.m_id) != materialIdAndEpd.end())
					continue;

				MatEpd &matEpd = materialIdAndEpd[mat.m_id];
				//check each material epd id
				for (auto idEpd : mat.m_idEpds) {
					if(idEpd == VICUS::INVALID_ID)
						continue;

					VICUS::EPDDataset epd = elementExists<VICUS::EPDDataset>(m_dbEPDs, idEpd,
																   mat.m_displayName.string(),
																   "EPD",
																   "material");

					//if we found the right dataset add values A1- A2
					if(epd.m_module == VICUS::EPDDataset::M_A1 ||
					   epd.m_module == VICUS::EPDDataset::M_A2 ||
					   epd.m_module == VICUS::EPDDataset::M_A1_A2||
					   epd.m_module == VICUS::EPDDataset::M_A3 ||
					   epd.m_module == VICUS::EPDDataset::M_A1_A3){
						//add all values in a category A
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdA.m_para[i].set(para.name,
															matEpd.m_epdA.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
					else if (epd.m_module == VICUS::EPDDataset::M_B6) {
						//add all values in a category B
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdB.m_para[i].set(para.name,
															matEpd.m_epdB.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
					else if (epd.m_module == VICUS::EPDDataset::M_C2 ||
							 epd.m_module == VICUS::EPDDataset::M_C2_C4 ||
							 epd.m_module == VICUS::EPDDataset::M_C3 ||
							 epd.m_module == VICUS::EPDDataset::M_C2_C3 ||
							 epd.m_module == VICUS::EPDDataset::M_C3_C4 ||
							 epd.m_module == VICUS::EPDDataset::M_C4) {
						//add all values in a category C
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdC.m_para[i].set(para.name,
															matEpd.m_epdC.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
					else if (epd.m_module == VICUS::EPDDataset::M_D) {
						//add all values in a category D
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							//...
							if(para.value != 0){
								matEpd.m_epdD.m_para[i].set(para.name,
															matEpd.m_epdD.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
				}
			}
		}
	}



	for (auto &e : compRes) {
		//Component result object
		LCAComponentResult &comp = e.second;
		unsigned int compId = e.first;

		//check if opaque construction is available
		if(m_dbComponents[compId].m_idConstruction != VICUS::INVALID_ID){
			const VICUS::Construction &constr = m_dbConstructions[m_dbComponents[compId].m_idConstruction];

			//get values for all material layers in each category of the lifecycle
			for(auto &l : constr.m_materialLayers){
				MatEpd &matEpd = materialIdAndEpd[l.m_idMaterial];
				double rho = m_dbOpaqueMaterials[l.m_idMaterial].m_para[VICUS::Material::P_Density].get_value("kg/m3");

//				addEpdMaterialToComponent(matEpd.m_epdA, comp, compResErsatz[compId],
//							   l.m_lifeCylce, l.m_thickness.get_value("m"),
//							   rho, 0, m_adjustment);

				addEpdMaterialToComponent(matEpd.m_epdB, comp, compResErsatz[compId],
							   0, l.m_thickness.get_value("m"),
							   rho, 1, m_adjustment);

//				addEpdMaterialToComponent(matEpd.m_epdC, comp, compResErsatz[compId],
//							   l.m_lifeCylce, l.m_thickness.get_value("m"),
//							   rho, 2, m_adjustment);

//				addEpdMaterialToComponent(matEpd.m_epdD, comp, compResErsatz[compId],
//							   l.m_lifeCylce, l.m_thickness.get_value("m"),
//							   rho, 3, m_adjustment);

			}

		}

	}
#endif
}

bool findUUID(QString uuid, unsigned int &id, const std::map<unsigned int, VICUS::EpdDataset> &db){
	for (auto o : db) {
		if(QString::compare(o.second.m_uuid, uuid,Qt::CaseInsensitive)){
			id = o.first;
			return true;
		}
	}
	return false;
}

void LCA::readDatabaseOekobautdat(const IBK::Path & filename)
{
#if 0
	FUNCID(LCA::readDatabaseOekobautdat);
//	IBK::FileReader fileR(filename);
//	IBK::FileReader::BOMType type = IBK::FileReader::getBOM(fileR.firstBytes(4));

	std::vector<std::string> lines;

	IBK::FileReader::readAll(filename, lines, std::vector<std::string>{"\n"});
#if _WIN32
	std::string testStr = IBK::ANSIToUTF8String(lines[0]);
#else
	std::string testStr = lines[0];
#endif

	std::map<unsigned int, VICUS::EPDDataset> &db = SVSettings::instance().m_dbEPDElements;

	/* falls eine datenbank existiert
	 * prüfen ob die daten sich geändert haben und evtl nachbessern
	 * ansonsten eintragen
	 * prüfkriterium ist die uuid
	*/

	//skip first line (Header)
	std::vector<std::string> tabElements;
	IBK::explode(testStr, tabElements, ";",IBK::EF_TrimTokens);
	size_t numberOfTabs = tabElements.size();
	for (size_t iLine=1; iLine<lines.size(); ++iLine) {
		tabElements.clear();
		IBK::explode(testStr, tabElements, ";",IBK::EF_TrimTokens);

		//skip lines that are not in the right manner
		if(numberOfTabs != tabElements.size())
			continue;

		VICUS::EPDDataset epd;
		epd.m_id = SVSettings::firstFreeId(db,1);
		epd.m_uuid = QString::fromStdString(tabElements[0]);

		std::string checkStr;

		//subtype
		checkStr = IBK::tolower_string(tabElements[5]);
		if(checkStr == "average dataset")
			epd.m_subtype = VICUS::EPDDataset::M_Average;
		else if(checkStr == "generic dataset")
			epd.m_subtype = VICUS::EPDDataset::M_Generic;
		else if(checkStr == "representative dataset")
			epd.m_subtype = VICUS::EPDDataset::M_Representative;
		else if(checkStr == "specific dataset")
			epd.m_subtype = VICUS::EPDDataset::M_Specific;
		else if(checkStr == "template dataset")
			epd.m_subtype = VICUS::EPDDataset::M_Template;
		else
			throw IBK::Exception(IBK::FormatString("No valid subtype of epd material '%1' available.").
								 arg(epd.m_displayName.toStdString()), FUNC_ID);
		epd.m_displayName = QString::fromStdString(tabElements[2]);
		epd.m_referenceQuantity = IBK::string2val<double>(tabElements[6]);

		//reference unit
		checkStr = IBK::tolower_string(tabElements[7]);
		if(checkStr == "kg")
			epd.m_referenceUnit = "kg";
		else if(checkStr == "qm")
			epd.m_referenceUnit = "m2";
		else if(checkStr == "pcs.")
			epd.m_referenceUnit = "pcs.";
		else if(checkStr == "m3")
			epd.m_referenceUnit = "m3";
		else
			throw IBK::Exception(IBK::FormatString("No valid reference unit for epd material '%1' available.")
								 .arg(epd.m_displayName.toStdString()), FUNC_ID);
		IBK::Unit("m2");
		epd.m_para[VICUS::EPDDataset::P_BasisWeight] = IBK::Parameter("BasisWeight", IBK::string2val<double>(tabElements[11]), "kg/m2");
		epd.m_para[VICUS::EPDDataset::P_Density] = IBK::Parameter("Density", IBK::string2val<double>(tabElements[12]), "kg/m3");

		//Categories
		checkStr = IBK::tolower_string(tabElements[17]);
		if(checkStr == "A1_A3")
			epd.m_category = VICUS::EPDDataset::C_A1_A3;
		else if(checkStr == "A1")
			epd.m_category = VICUS::EPDDataset::C_A1;
		else if(checkStr == "A2")
			epd.m_category = VICUS::EPDDataset::C_A2;
		else if(checkStr == "A3")
			epd.m_category = VICUS::EPDDataset::C_A3;
		else if(checkStr == "A1_A2")
			epd.m_category = VICUS::EPDDataset::C_A1_A2;
		else if(checkStr == "B")
			epd.m_category = VICUS::EPDDataset::C_B6;
		else if(checkStr == "C2")
			epd.m_category = VICUS::EPDDataset::C_C2;
		else if(checkStr == "C2_C3")
			epd.m_category = VICUS::EPDDataset::C_C2_3;
		else if(checkStr == "C2_C4")
			epd.m_category = VICUS::EPDDataset::C_C2_C4;
		else if(checkStr == "C3")
			epd.m_category = VICUS::EPDDataset::C_C3;
		else if(checkStr == "C3_C4")
			epd.m_category = VICUS::EPDDataset::C_C3_C4;
		else if(checkStr == "C4")
			epd.m_category = VICUS::EPDDataset::C_C4;
		else if(checkStr == "D")
			epd.m_category = VICUS::EPDDataset::C_D;
		else
			throw IBK::Exception(IBK::FormatString("No valid category for epd material '%1' given.")
								 .arg(epd.m_displayName.toStdString()), FUNC_ID);
		epd.m_para[VICUS::EPDDataset::P_GWP] = IBK::Parameter("GWP", IBK::string2val<double>(tabElements[18]), "kg");
		epd.m_para[VICUS::EPDDataset::P_ODP] = IBK::Parameter("ODP", IBK::string2val<double>(tabElements[19]), "kg");
		epd.m_para[VICUS::EPDDataset::P_POCP] = IBK::Parameter("POCP", IBK::string2val<double>(tabElements[20]), "kg");
		epd.m_para[VICUS::EPDDataset::P_AP] = IBK::Parameter("AP", IBK::string2val<double>(tabElements[21]), "kg");
		epd.m_para[VICUS::EPDDataset::P_EP] = IBK::Parameter("EP", IBK::string2val<double>(tabElements[22]), "kg");
		epd.m_para[VICUS::EPDDataset::P_PERT] = IBK::Parameter("PERT", IBK::string2val<double>(tabElements[27]), "W/mK");
		epd.m_para[VICUS::EPDDataset::P_PENRT] = IBK::Parameter("PENRT", IBK::string2val<double>(tabElements[30]), "W/mK");

		//created a epd dataset

		//check if epd exists
		unsigned int id;
		if(findUUID(epd.m_uuid, id, db)){
			///TODO Mira
			/// behavelikes
			if(epd.behavesLike(db[id]))
				//wenn Datenbanken übereinstimmen, nimm die bereits existierende,
				//wenn Datenbanken nicht übereinstimmen und eine neue existiert, nimm neue
				//wenn keine Datenbank existiert, nimm neue
				;

		}
	}
#endif
}

}
