#ifndef SVLCAH
#define SVLCAH

#include <VICUS_Building.h>
#include <VICUS_EPDDataset.h>


namespace SV {

class SVLCA
{
public:

	/*! Calculate lca. */
	void calculateLCA();

	/*! TODO Andreas das ist noch falsch ie bekomm ich hier das gebäude*/
	VICUS::Building							m_building;

	/*! EPD results for hole building. */
	VICUS::EPDCategroySet					m_results;							// XML:E

};
}

#endif // SVLCAH
