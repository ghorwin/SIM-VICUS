#include "NANDRAD_HydraulicNetworkHeatExchange.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {


HydraulicNetworkHeatExchange::HydraulicNetworkHeatExchange()
{

}

void HydraulicNetworkHeatExchange::checkParameters(const HydraulicNetworkComponent &comp,
												   const std::map<std::string, IBK::Path> &placeholders)
{
	FUNCID(HydraulicNetworkHeatExchange::checkParameters);

	// check parameters required for thermal balances/heat exchange
	// Note: We ONLY check the required PARAMETERS here! Wether this is a correct heat exchange type or not is
	//       checked in the HydraulicComponent!
	//       Also, we do not check for implemented combinations of model type and heat exchange type - this is handled
	//       when the models are instantiated.

	bool heatExchangeDataFileMustExist = false;

	try {
		// decide which heat exchange is chosen
		switch(m_type) {

			case T_AmbientTemperatureConstant: {
					// check temperature
					m_para[P_AmbientTemperature].checkedValue("AmbientTemperature", "C", "C", -200.0, true, std::numeric_limits<double>::max(), true,
												   "Ambient temperature must be >= -200 C.");
					// check for existance of external heat transfer coefficient (the parameter itself was checked already by the component)
					m_para[P_ExternalHeatTransferCoefficient].checkedValue("ExternalHeatTransferCoefficient",
																				"W/m2K", "W/m2K", 0, false,
																				std::numeric_limits<double>::max(),
																				true, nullptr);
			} break;

			case T_HeatLossConstant: {
				// check heat flux
				m_para[P_HeatLoss].checkedValue("HeatLoss", "W", "W",
							std::numeric_limits<double>::lowest(), false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case T_HeatExchangeWithZoneTemperature: {
				// check for zone id
				if (m_intPara[IP_ZoneId].name.empty())
					throw IBK::Exception(IBK::FormatString("Missing IntParameter 'ZoneId'."), FUNC_ID);

				// check for existance of external heat transfer coefficient (the parameter itself was checked already by the component)
				if (m_para[P_ExternalHeatTransferCoefficient].name.empty()){
					throw IBK::Exception(IBK::FormatString("Missing parameter %1 in definition of network component '%2' (#%3).")
								.arg(KeywordList::Keyword("HydraulicNetworkHeatExchange::para_t",
								P_ExternalHeatTransferCoefficient))
								.arg(comp.m_displayName).arg(comp.m_id), FUNC_ID);
				}
			} break;

			case T_HeatLossSpline:
			case T_AmbientTemperatureSpline:{
				heatExchangeDataFileMustExist = true;
				break;
			}

			case T_HeatExchangeWithFMUTemperature: {
				throw IBK::Exception(IBK::FormatString("Heat exchange type 'HeatExchangeWithFMUTemperature' is not supported, yet!"),
							FUNC_ID);
			}
			case NUM_T:
				// No thermal exchange, nothing to initialize
			break;
		} // switch
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for heat exchange model %1.")
			 .arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType", m_type)),
			 FUNC_ID);
	}


	// check and read csv file
	if (heatExchangeDataFileMustExist){

		// replace place holders
		m_spline.m_tsvFile = m_spline.m_tsvFile.withReplacedPlaceholders(placeholders);

		try {
			if (m_type == T_AmbientTemperatureSpline){
				//  check the spline and convert it to base units automatically
				m_spline.checkAndInitialize("Spline", IBK::Unit("s"), IBK::Unit("K"),
														IBK::Unit("K"), 0, false, std::numeric_limits<double>::max(), false,
														"Temperature must be > 0 K.");
			}

			else if (m_type == T_HeatLossSpline){
				//  check the spline and convert it to base units automatically
				m_spline.checkAndInitialize("Spline", IBK::Unit("s"), IBK::Unit("J/s"),
														IBK::Unit("J/s"), std::numeric_limits<double>::lowest(), false,
														std::numeric_limits<double>::max(), false,
														nullptr);
			}
		} catch (IBK::Exception &ex) {
			if (m_spline.m_name.empty())
				throw IBK::Exception(ex, IBK::FormatString("Error initializing spline data."), FUNC_ID);
			else
				throw IBK::Exception(ex, IBK::FormatString("Error initializing spline '%1'.").arg(m_spline.m_name), FUNC_ID);
		}

	}

}



std::vector<unsigned int> NANDRAD::HydraulicNetworkHeatExchange::availableHeatExchangeTypes(const NANDRAD::HydraulicNetworkComponent::ModelType modelType)
{
	switch (modelType) {
		case HydraulicNetworkComponent::MT_SimplePipe:
			return {T_AmbientTemperatureConstant, T_AmbientTemperatureSpline, T_HeatLossConstant, T_HeatLossSpline};
		case HydraulicNetworkComponent::MT_DynamicPipe:
			return {T_AmbientTemperatureConstant, T_AmbientTemperatureSpline, T_HeatLossConstant, T_HeatLossSpline};
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnot:
		case HydraulicNetworkComponent::MT_HeatExchanger:
			return {T_HeatLossConstant, T_HeatLossSpline};
		case HydraulicNetworkComponent::MT_ConstantPressurePump:
		case HydraulicNetworkComponent::NUM_MT: ;
	}
	return {};
}



} // namespace NANDRAD
