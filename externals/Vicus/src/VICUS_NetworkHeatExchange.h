#ifndef NETWORKHEATEXCHANGE_H
#define NETWORKHEATEXCHANGE_H

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include "NANDRAD_LinearSplineParameter.h"

namespace VICUS {

class NetworkHeatExchange
{
public:

	VICUS_READWRITE

	NetworkHeatExchange();

	bool operator!=(const NetworkHeatExchange &other) const;

	/*! Defines the type of heat exchange */
	enum Type {
		HT_TemperatureConstant,				// Keyword: TemperatureConstant					'Constant temperature'
		HT_HeatFluxConstant,				// Keyword: HeatFluxConstant					'Constant heat flux'
		HT_HeatFluxDataFile,				// Keyword: HeatFluxDataFile					'Heat flux from data file'
		HT_TemperatureDataFile,				// Keyword: TemperatureDataFile					'Temperature from data file'
		HT_HeatExchangeWithZoneTemperature,	// Keyword: HeatExchangeWithZoneTemperature		'Heat exchange with zone'
		HT_HeatExchangeWithFMUTemperature,	// Keyword: HeatExchangeWithFMUTemperature		'Heat exchange with FMU which requires temperature and provides heat flux'
		NUM_HT
	};


	enum Parameter{
		P_AmbientTemperature,	// Keyword: AmbientTemperature			[C]		'Ambient temperature'
		P_HeatLoss,				// Keyword: HeatLoss					[W]		'Heat loss'
		NUM_P
	};

	/*! Integer/whole number parameters. */
	enum IntParameter {
		IP_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
		NUM_IP
	};


	Type							m_type	= NUM_HT;						// XML:E

	IBK::Parameter					m_para[NUM_P];							// XML:E

	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];						// XML:E

	NANDRAD::LinearSplineParameter	m_spline;								// XML:E

};

} // namespace VICUS

#endif // NETWORKHEATEXCHANGE_H
