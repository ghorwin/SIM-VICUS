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

	HydraulicNetworkHeatExchange() {
		for (unsigned int & i : m_idReferences) i = INVALID_ID;
	}

	NANDRAD_READWRITE

	/*! Tests all parameter and initializes linear spline parameters for calculation,
		including reading of potentially referenced TSV files.
	*/
	void checkParameters(const std::map<std::string, IBK::Path> &placeholders);

	/*! Defines the type of heat exchange */
	enum ModelType {
		T_TemperatureConstant,				// Keyword: TemperatureConstant			'Difference to constant temperature'
		T_TemperatureSpline,				// Keyword: TemperatureSpline			'Difference to time-dependent temperature from spline'
		T_HeatLossConstant,					// Keyword: HeatLossConstant			'Constant heat loss'
		T_HeatLossSpline,					// Keyword: HeatLossSpline				'Heat loss from spline'
		T_HeatLossIdealHeatPump,			// Keyword: HeatLossIdealHeatPump		'Heat loss from ideal heat pump model'
		T_TemperatureZone,					// Keyword: TemperatureZone				'Difference to zone air temperature'
		T_TemperatureFMUInterface,			// Keyword: TemperatureFMUInterface		'Difference to temperature from FMU interface, provided heat flux to FMU'
		NUM_T
	};

	/*! Parameters for the element . */
	enum para_t {
		P_Temperature,						// Keyword: Temperature					[C]		'Temperature for heat exchange'
		P_HeatLoss,							// Keyword: HeatLoss							[W]		'Constant heat flux out of the element (heat loss)'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary'
		NUM_P
	};

	/*! Spline parameter as functions of time with the implied assumption that
		t = 0 means begin of simulation.
		TODO Andreas + Anne + Hauke
	*/
	enum splinePara_t {
		SPL_Temperature,					// Keyword: Temperature							[C]		'Temperature for heat exchange'
		SPL_HeatLoss,						// Keyword: HeatLoss							[W]		'Constant heat flux out of the element (heat loss)'
		NUM_SPL
	};

	/*! Integer/whole number parameters. */
	enum References {
		ID_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
		NUM_ID
	};

	/*! Model/type of heat exchange. */
	ModelType						m_modelType = NUM_T;									// XML:A:required

	// TODO: Correct code generator and remove entry
	unsigned int					m_fixMe = 999;											// XML:E

	/*! Integer parameters. */
	IDType							m_idReferences[NUM_ID];									// XML:E

	/*! Parameter */
	IBK::Parameter					m_para[NUM_P];											// XML:E

	/*! Time-series of heat flux or temperature (can be spline or tsv-file).
		Note: the XML tag name is always the same "HeatExchangeSpline", yet the content (and physical units)
		differ depending on selected heat exchange type.
	*/
	LinearSplineParameter			m_splPara[NUM_SPL];										// XML:E


	// *** Static functions ***

	static std::vector<unsigned int> availableHeatExchangeTypes(const HydraulicNetworkComponent::ModelType modelType);

};


} // namespace NANDRAD

#endif // HydraulicNetworkHeatExchangeH
