#include "NANDRAD_HydraulicNetworkHeatExchange.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {


void HydraulicNetworkHeatExchange::checkParameters(const std::map<std::string, IBK::Path> &placeholders) {
	FUNCID(HydraulicNetworkHeatExchange::checkParameters);

	// check parameters required for thermal balances/heat exchange
	// Note: We ONLY check the required PARAMETERS here! We do not check for implemented combinations of model type
	//       and heat exchange type - this is handled when the models are instantiated.

	try {
		// decide which heat exchange is chosen
		switch (m_modelType) {
			case T_TemperatureConstant: {
				// check temperature
				m_para[P_Temperature].checkedValue("AmbientTemperature", "C", "C", -200.0, true, std::numeric_limits<double>::max(), true,
												   "Ambient temperature must be >= -200 C.");
				// check for external heat transfer coefficient
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

			case T_TemperatureZone: {
				// check for zone id
				if (m_idReferences[ID_ZoneId] == INVALID_ID)
					throw IBK::Exception(IBK::FormatString("Missing IntParameter 'ZoneId'."), FUNC_ID);

				// check for external heat transfer coefficient
				m_para[P_ExternalHeatTransferCoefficient].checkedValue("ExternalHeatTransferCoefficient",
																	   "W/m2K", "W/m2K", 0, false,
																	   std::numeric_limits<double>::max(),
																	   true, nullptr);
			} break;

			case T_HeatLossSpline: {
				// replace place holders
				m_splPara[SPL_HeatLoss].m_tsvFile = m_splPara[SPL_HeatLoss].m_tsvFile.withReplacedPlaceholders(placeholders);
				try {
					//  check the spline and convert it to base units automatically
					m_splPara[SPL_HeatLoss].checkAndInitialize("HeatLoss", IBK::Unit("s"), IBK::Unit("J/s"),
															   IBK::Unit("J/s"), std::numeric_limits<double>::lowest(), false,
															   std::numeric_limits<double>::max(), false,
															   nullptr);
				} catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error initializing spline 'HeatLoss'."), FUNC_ID);
				}

			} break;

			case T_TemperatureSpline: {
				try {
					//  check the spline and convert it to base units automatically
					m_splPara[SPL_Temperature].checkAndInitialize("Temperature", IBK::Unit("s"), IBK::Unit("K"),
																  IBK::Unit("K"), 0, false, std::numeric_limits<double>::max(), false,
																  "Temperature must be > 0 K.");
				} catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error initializing spline 'Temperature'."), FUNC_ID);
				}

				// replace place holders
				m_splPara[SPL_Temperature].m_tsvFile = m_splPara[SPL_Temperature].m_tsvFile.withReplacedPlaceholders(placeholders);
				// check for external heat transfer coefficient
				m_para[P_ExternalHeatTransferCoefficient].checkedValue("ExternalHeatTransferCoefficient",
																	   "W/m2K", "W/m2K", 0, false,
																	   std::numeric_limits<double>::max(),
																	   true, nullptr);
			} break;

			case T_TemperatureFMUInterface:
				// TODO : Andreas
				throw IBK::Exception(IBK::FormatString("Heat exchange type 'TemperatureFMUInterface' is not supported, yet!"), FUNC_ID);

			case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossIdealHeatPump:
				// TODO : Hauke
				throw IBK::Exception(IBK::FormatString("Heat exchange type 'HeatLossIdealHeatPump' is not supported, yet!"), FUNC_ID);


			case NUM_T:
				// No thermal exchange, nothing to initialize
			break;

		} // switch

	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for heat exchange model %1.")
			 .arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType", m_modelType)),
			 FUNC_ID);
	}
}



std::vector<unsigned int> NANDRAD::HydraulicNetworkHeatExchange::availableHeatExchangeTypes(
		const NANDRAD::HydraulicNetworkComponent::ModelType modelType)
{
	switch (modelType) {
		case HydraulicNetworkComponent::MT_SimplePipe:
			return {T_TemperatureConstant, T_TemperatureSpline, T_HeatLossConstant, T_HeatLossSpline};
		case HydraulicNetworkComponent::MT_DynamicPipe:
			return {T_TemperatureConstant, T_TemperatureSpline, T_HeatLossConstant, T_HeatLossSpline};
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnot:
		case HydraulicNetworkComponent::MT_HeatExchanger:
			return {T_HeatLossConstant, T_HeatLossSpline};
		case HydraulicNetworkComponent::MT_ConstantPressurePump:
		case HydraulicNetworkComponent::NUM_MT: ;
	}
	return {};
}



} // namespace NANDRAD
