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

#ifndef SVLCAH
#define SVLCAH

#include <VICUS_Building.h>
#include <VICUS_EpdDataset.h>


namespace SV {

class LCA
{
public:


	// results
	// cat set A - D with the results for this specific construction not multiplied by surface area
	struct LCAComponentResult{

		/*!
			adds all calculated parametervalus to the component (if called in combination with comp)
			depending on the category of the lifecycle.
		*/
//		void addValue(unsigned int catIdx, VICUS::EpdDataset::para_t paraIdx, const IBK::Parameter &para){
//			switch (catIdx) {
//				case 0:
//					m_epdA.m_para[paraIdx].set(para.name, m_epdA.m_para[paraIdx].get_value(para.unit())
//											   + para.get_value(para.unit()), para.unit());
//					break;
//				case 1:
//					m_epdB.m_para[paraIdx].set(para.name, m_epdB.m_para[paraIdx].get_value(para.unit())
//											   + para.get_value(para.unit()), para.unit());
//					break;
//				case 2:
//					m_epdC.m_para[paraIdx].set(para.name, m_epdC.m_para[paraIdx].get_value(para.unit())
//											   + para.get_value(para.unit()), para.unit());
//					break;
//				case 3:
//					m_epdD.m_para[paraIdx].set(para.name, m_epdD.m_para[paraIdx].get_value(para.unit())
//											   + para.get_value(para.unit()), para.unit());
//					break;
//				default:
//					break;
//			}
//		}


		VICUS::EpdDataset m_epdA;
		VICUS::EpdDataset m_epdB;
		VICUS::EpdDataset m_epdC;
		VICUS::EpdDataset m_epdD;		// user epd of the specific construction
		 double									m_area;				// in m2
	};

	/*! Calculate lca (Life Cycle Assessment).
		This function calculates all the EPDs for the whole lifecycle in the categories which are filled with Data.
		This is mostly the case for the used categories. Others exist, but are rarely populated with datasets. That is why they are not considered until now.
	*/
	void calculateLCA();


	/*! Imports the database Oekobaudat from the ministery BMI.
		All values required at the present time are imported. */
	void readDatabaseOekobautdat(const IBK::Path &filename);


	//double adjustmentReferenceUnit(const QString &refQ);

	/*! TODO Dirk, LCA -> Milestone anlegen und dort in Tickets/Discussion besprechen. */
	VICUS::Building							m_building;

	/*! EPD results for
		//for-Schleife über alle Materialienhole building. */
	//VICUS::EPDCategroySet					m_results;							// XML:E

	/*! the factor 1.2 is according to the use of simplified procedure. */
	double									m_adjustment = 1.2;					// XML:E

};
}

#endif // SVLCAH
