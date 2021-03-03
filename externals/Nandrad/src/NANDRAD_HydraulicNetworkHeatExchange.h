#ifndef HYDRAULICNETWORKHEATEXCHANGE_H
#define HYDRAULICNETWORKHEATEXCHANGE_H

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#include <IBK_IntPara.h>

namespace NANDRAD {


class HydraulicNetworkHeatExchange
{
public:
	HydraulicNetworkHeatExchange();


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	void checkParameters(const HydraulicNetworkComponent &comp,
						 const std::map<std::string, IBK::Path> &placeholders);

	/*! Defines the type of heat exchange */
	enum Type {
		T_TemperatureConstant,				// Keyword: TemperatureConstant					'Constant temperature'
		T_HeatFluxConstant,					// Keyword: HeatFluxConstant					'Constant heat flux'
		T_HeatFluxDataFile,					// Keyword: HeatFluxDataFile					'Heat flux from data file'
		T_TemperatureDataFile,				// Keyword: TemperatureDataFile					'Temperature from data file'
		T_HeatExchangeWithZoneTemperature,	// Keyword: HeatExchangeWithZoneTemperature		'Heat exchange with zone'
		T_HeatExchangeWithFMUTemperature,	// Keyword: HeatExchangeWithFMUTemperature		'Heat exchange with FMU which requires temperature and provides heat flux'
		NUM_T
	};

	/*! Parameters for the element . */
	enum para_t {
		P_AmbientTemperature,				// Keyword: AmbientTemperature						[C]		'Temperature for heat exchange'
		P_HeatLoss,							// Keyword: HeatLoss								[W]		'Constant heat flux out of the element (heat loss)'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary'
		NUM_P
	};

	/*! Integer/whole number parameters. */
	enum intPara_t {
		IP_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
		NUM_IP
	};

	/*! Type of interface to external data or model */
	Type							m_type = NUM_T;											// XML:E

	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];										// XML:E

	/*! Parameter */
	IBK::Parameter					m_para[NUM_P];											// XML:E

	/*! Time-series of heat flux or temperature (can be spline or tsv-file).
		Note: the XML tag name is always the same "HeatExchangeSpline", yet the content (and physical units)
		differ depending on selected heat exchange type.
	*/
	LinearSplineParameter			m_spline;												// XML:E


	// *** Static functions ***

	static std::vector<unsigned int> availableHeatExchangeTypes(const HydraulicNetworkComponent::ModelType modelType);

};


} // namespace NANDRAD

#endif // HYDRAULICNETWORKHEATEXCHANGE_H
