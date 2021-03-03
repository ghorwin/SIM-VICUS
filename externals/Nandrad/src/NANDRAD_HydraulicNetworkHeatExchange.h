#ifndef HydraulicNetworkHeatExchangeH
#define HydraulicNetworkHeatExchangeH

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#include <IBK_IntPara.h>

namespace NANDRAD {


/*! Encapsulates all data defining heat exchange between flow elements and
	the environment or other models/elements.

	Definition of heat exchange is done in each flow element definition. If missing, the flow
	element is treated as adiabat.
*/
class HydraulicNetworkHeatExchange {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	/*! Tests all parameter and initializes linear spline parameters for calculation,
		including reading of potentially referenced TSV files.
	*/
	void checkParameters(const HydraulicNetworkComponent &comp,
						 const std::map<std::string, IBK::Path> &placeholders);

	/*! Defines the type of heat exchange */
	enum ModelType {
		T_AmbientTemperatureConstant,		// Keyword: AmbientTemperatureConstant			'Constant ambient temperature'
		T_AmbientTemperatureSpline,			// Keyword: AmbientTemperatureSpline			'Ambient Temperature from spline'
		T_HeatLossConstant,					// Keyword: HeatLossConstant					'Constant heat loss'
		T_HeatLossSpline,					// Keyword: HeatLossSpline						'Heat loss from spline'
		T_HeatExchangeWithZoneTemperature,	// Keyword: HeatExchangeWithZoneTemperature		'Heat exchange with zone'
		T_HeatExchangeWithFMUTemperature,	// Keyword: HeatExchangeWithFMUTemperature		'Heat exchange with FMU which requires temperature and provides heat flux'
		NUM_T
	};

	/*! Parameters for the element . */
	enum para_t {
		P_AmbientTemperature,				// Keyword: AmbientTemperature					[C]		'Temperature for heat exchange'
		P_HeatLoss,							// Keyword: HeatLoss							[W]		'Constant heat flux out of the element (heat loss)'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary'
		NUM_P
	};

	/*! Integer/whole number parameters. */
	enum intPara_t {
		IP_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
		NUM_IP
	};

	/*! Model/type of heat exchange. */
	ModelType						m_modelType = NUM_T;									// XML:A:required

	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];										// XML:E

	/*! Parameter */
	IBK::Parameter					m_para[NUM_P];											// XML:E

	/*! Time-series of heat flux or temperature (can be spline or tsv-file).
		Note: the XML tag name is always the same "HeatExchangeSpline", yet the content (and physical units)
		differ depending on selected heat exchange type.
	*/
	LinearSplineParameter			m_heatExchangeSpline;									// XML:E


	// *** Static functions ***

	static std::vector<unsigned int> availableHeatExchangeTypes(const HydraulicNetworkComponent::ModelType modelType);

};


} // namespace NANDRAD

#endif // HydraulicNetworkHeatExchangeH
