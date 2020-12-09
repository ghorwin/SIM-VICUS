#include "SVLCA.h"

#include "SVSettings.h"
#include <VICUS_EPDCategroySet.h>

namespace SV {

template <typename T>
T elementExists(std::map<unsigned int, T> database, unsigned int id, std::string displayName,
				std::string idName, std::string objectType){
	FUNCID(SVLCA::elementExists);
	if(database.find(id) == database.end())
		IBK::Exception(IBK::FormatString("%3 id %1 of %4 '%2' is not contained in database. ")
					   .arg(id).arg(displayName).arg(idName).arg(objectType), FUNC_ID);
	return  database[id];
}

void SVLCA::calculateLCA()
{
	FUNCID(SVLCA::calculateLCA);

	/*! Summarize all components with all constructions and material layers.
		Categorize all construction with their surface areas.
	*/

	/* Annahmen: diese Strukturen müssen umbenannt werden. */
	std::map<unsigned int, VICUS::Component>				m_dbComponents;
	std::map<unsigned int, VICUS::Construction>				m_dbConstructions;
	std::map<unsigned int, VICUS::Material>					m_dbOpaqueMaterials;
	std::map<unsigned int, VICUS::EPDDataset>				m_dbEPDs;

	// results
	// cat set A - D with the results for this specific construction not multiplied by surface area
	struct LCAComponentResult{
		 std::vector<VICUS::EPDDataset>			m_epdDataset;		// user epd of the specific construction
		 double									m_area;				// in m2
	};

	struct MatEpd{
		VICUS::EPDDataset m_epdA;
		VICUS::EPDDataset m_epdB;
		VICUS::EPDDataset m_epdC;
		VICUS::EPDDataset m_epdD;
	};

	std::map<unsigned int, LCAComponentResult>		compRes;

	//holds the data for each material
	std::map<unsigned int, MatEpd>					materialIdAndEpd;


	/* Calculate all surface areas according to all components. */
	for (auto &bl : m_building.m_buildingLevels) {
		for (auto &r : bl.m_rooms) {
			for (auto &s : r.m_surfaces) {
				const VICUS::Surface &surf = s;

				//get component
				VICUS::Component comp = elementExists<VICUS::Component>(m_dbComponents, s.m_componentId,
												s.m_displayName.toStdString(),"Component", "surface");
				//save surface area
				compRes[comp.m_id].m_area += surf.m_geometry.area();
			}
		}
	}

	//calculate all lca for each component
	for (auto &c : compRes) {
		const VICUS::Component &comp=m_dbComponents[c.first];

		//opaque construction
		if(comp.m_idOpaqueConstruction != VICUS::INVALID_ID){
			//get construction
			VICUS::Construction constr =
					elementExists<VICUS::Construction>(m_dbConstructions, comp.m_idOpaqueConstruction,
													   comp.m_displayName.toStdString(),"Construction",
													   "component");

			//calc each construction
			for(auto l : constr.m_materialLayers){
				//double thickness = l.m_thickness.get_value("m");	//thickness in m
				//check if material exists
				VICUS::Material mat =
						elementExists<VICUS::Material>(m_dbOpaqueMaterials, l.m_matId,
														   constr.m_displayName.toStdString(),
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
																   mat.m_displayName.toStdString(),
																   "EPD",
																   "material");
					/// TODO Mira alle Kategorien durchprogrammieren
					//if we found the right dataset add values
					if(epd.m_category == VICUS::EPDDataset::C_A1 ||
							epd.m_category == VICUS::EPDDataset::C_A2){
						//add all values in a category e.g. A, B, ...
						for (unsigned int i=0;i< VICUS::EPDDataset::NUM_P; ++i) {
							IBK::Parameter para = epd.m_para[i];
							if(para.value != 0){
								matEpd.m_epdA.m_para[i].set(para.name,
															matEpd.m_epdA.m_para[i].get_value(para.unit())
															+ para.get_value(para.unit()),
															para.unit());
							}
						}
					}
					else if (true) {

					}
				}
			}
		}
	}
	/*
		schleife über alle components
		schleife über constructions

		berechnung mit erneuerungszyklen und dicke um konstruktionsergebnisse abzubilden

		multiplikation mit fläche

		ausgabe aufbreiten
		mit tabelle ...
	*/
}

}
