#ifndef NETWORKHEATEXCHANGE_H
#define NETWORKHEATEXCHANGE_H

#include <IBK_Parameter.h>
#include <IBK_Path.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class NetworkHeatExchange
{
public:

	VICUS_READWRITE

	NetworkHeatExchange();

	bool operator!=(const NetworkHeatExchange &other) const;

	enum Parameter{
		P_AmbientTemperature,	// Keyword: AmbientTemperature			[C]		'Ambient temperature'
		P_HeatLoss,				// Keyword: HeatLoss					[W]		'Heat loss'
		NUM_P
	};

	IBK::Parameter			m_para[NUM_P];				// XML:E

	IBK::Path				m_dataFile;					// XML:E

	IBK::Path				m_fmuFile;					// XML:E
};

} // namespace VICUS

#endif // NETWORKHEATEXCHANGE_H
