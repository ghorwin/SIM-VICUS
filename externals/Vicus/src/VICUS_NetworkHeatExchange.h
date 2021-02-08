#ifndef NETWORKHEATEXCHANGE_H
#define NETWORKHEATEXCHANGE_H

#include <IBK_Parameter.h>
#include <IBK_Path.h>

namespace VICUS {

class NetworkHeatExchange
{
public:
	NetworkHeatExchange();

	enum Parameter{
		P_Temperature,
		P_HeatFlux,
		NUM_P
	};

	IBK::Parameter			m_para[NUM_P];

	IBK::Path				m_dataFile;

	IBK::Path				m_fmuFile;
};

} // namespace VICUS

#endif // NETWORKHEATEXCHANGE_H
