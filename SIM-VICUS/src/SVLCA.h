#ifndef SVLCAH
#define SVLCAH

#include <VICUS_Building.h>
#include <VICUS_EPDDataset.h>


namespace SV {

class SVLCA
{
public:
	/*! Calculate lca.
		This function calculates all the EPDs for the whole lifecycle in the categories which are filled with Data.
		This is mostly the case for the used categories. Others exist, but are rarely populated with datasets. That is why they are not considered until now.
	*/
	void calculateLCA();

	/*! TODO Andreas das ist noch falsch ie bekomm ich hier das gebäude*/
	VICUS::Building							m_building;

	/*! EPD results for
		//for-Schleife über alle Materialienhole building. */
	VICUS::EPDCategroySet					m_results;							// XML:E

};
}

#endif // SVLCAH
