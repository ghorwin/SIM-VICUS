#ifndef NANDRAD_HYDRAULICNETWORKNODEH
#define NANDRAD_HYDRAULICNETWORKNODEH

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

class HydraulicNetworkNode
{
public:

	HydraulicNetworkNode();

	HydraulicNetworkNode(unsigned int id, const double &height);

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	IDType				m_id		= NANDRAD::INVALID_ID;				// XML:A:required

	double				m_height	= 0;								// XML:A
};

} // namespace NANDRAD

#endif // NANDRAD_HYDRAULICNETWORKNODEH
