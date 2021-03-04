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
	enum ModelType {
		T_AmbientTemperatureConstant,		// Keyword: AmbientTemperatureConstant			'Constant ambient temperature'
		T_AmbientTemperatureSpline,			// Keyword: AmbientTemperatureSpline			'Ambient Temperature from spline'
		T_HeatLossConstant,					// Keyword: HeatLossConstant					'Constant heat loss'
		T_HeatLossSpline,					// Keyword: HeatLossSpline						'Heat loss from spline'
		HT_HeatExchangeWithZoneTemperature,	// Keyword: HeatExchangeWithZoneTemperature		'Heat exchange with zone'
		HT_HeatExchangeWithFMUTemperature,	// Keyword: HeatExchangeWithFMUTemperature		'Heat exchange with FMU which requires temperature and provides heat flux'
		NUM_HT
	};


	enum Parameter{
		P_AmbientTemperature,				// Keyword: AmbientTemperature					[C]		'Ambient temperature'
		P_HeatLoss,							// Keyword: HeatLoss							[W]		'Heat loss'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary'
		NUM_P
	};

	/*! Integer/whole number parameters. */
	enum IntParameter {
		IP_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
		NUM_IP
	};


	ModelType							m_modelType	= NUM_HT;						// XML:E

	IBK::Parameter					m_para[NUM_P];							// XML:E

	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];						// XML:E

	NANDRAD::LinearSplineParameter	m_heatExchangeSpline;								// XML:E

};

} // namespace VICUS

#endif // NETWORKHEATEXCHANGE_H
