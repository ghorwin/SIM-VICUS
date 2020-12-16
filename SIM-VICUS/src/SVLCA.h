#ifndef SVLCAH
#define SVLCAH

#include <VICUS_Building.h>
#include <VICUS_EPDDataset.h>


namespace SV {

class LCA
{
public:


	// results
	// cat set A - D with the results for this specific construction not multiplied by surface area
	struct LCAComponentResult{

		/*! adds all calculated parametervalus to the component (if called in combination with comp)
			depending on the category of the lifecycle.
		*/
		void addValue(unsigned int catIdx, VICUS::EPDDataset::para_t paraIdx, const IBK::Parameter &para){
			switch (catIdx) {
				case 0:
					m_epdA.m_para[paraIdx].set(para.name, m_epdA.m_para[paraIdx].get_value(para.unit())
											   + para.get_value(para.unit()), para.unit());
					break;
				case 1:
					m_epdB.m_para[paraIdx].set(para.name, m_epdB.m_para[paraIdx].get_value(para.unit())
											   + para.get_value(para.unit()), para.unit());
					break;
				case 2:
					m_epdC.m_para[paraIdx].set(para.name, m_epdC.m_para[paraIdx].get_value(para.unit())
											   + para.get_value(para.unit()), para.unit());
					break;
				case 3:
					m_epdD.m_para[paraIdx].set(para.name, m_epdD.m_para[paraIdx].get_value(para.unit())
											   + para.get_value(para.unit()), para.unit());
					break;
				default:
					break;
			}
		}


		VICUS::EPDDataset m_epdA;
		VICUS::EPDDataset m_epdB;
		VICUS::EPDDataset m_epdC;
		VICUS::EPDDataset m_epdD;		// user epd of the specific construction
		 double									m_area;				// in m2
	};

	/*! Calculate lca.
		This function calculates all the EPDs for the whole lifecycle in the categories which are filled with Data.
		This is mostly the case for the used categories. Others exist, but are rarely populated with datasets. That is why they are not considered until now.
	*/
	void calculateLCA();


	/*! TODO MIRA */
	void readDatabaseOekobautdat(const IBK::Path &filename);


	//double adjustmentReferenceUnit(const QString &refQ);

	/*! TODO Andreas das ist noch falsch ie bekomm ich hier das gebäude*/
	VICUS::Building							m_building;

	/*! EPD results for
		//for-Schleife über alle Materialienhole building. */
	//VICUS::EPDCategroySet					m_results;							// XML:E

	/*! the factor 1.2 is according to the use of simplified procedure. */
	double									m_adjustment = 1.2;					// XML:E

};
}

#endif // SVLCAH
