#include "NANDRAD_HydraulicNetworkElement.h"

#include "NANDRAD_HydraulicNetwork.h"

#include <NANDRAD_KeywordList.h>

#include <IBK_UnitList.h>
#include <IBK_CSVReader.h>

namespace NANDRAD {

HydraulicNetworkElement::HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
						unsigned int componentId, unsigned int pipeID, double length) :
	m_id(id),
	m_inletNodeId(inletNodeId),
	m_outletNodeId(outletNodeId),
	m_componentId(componentId),
	m_pipePropertiesId(pipeID)
{
	KeywordList::setParameter(m_para, "HydraulicNetworkElement::para_t", P_Length, length);
}


void HydraulicNetworkElement::checkParameters(const HydraulicNetwork & nw,
											  const std::map<std::string, IBK::Path> &placeholders) {
	FUNCID(HydraulicNetworkElement::checkParameters);

	// retrieve network component
	std::vector<HydraulicNetworkComponent>::const_iterator coit =
			std::find(nw.m_components.begin(), nw.m_components.end(), m_componentId);
	if (coit == nw.m_components.end()) {
		throw IBK::Exception(IBK::FormatString("Missing component reference to id%1.")
							 .arg(m_componentId), FUNC_ID);
	}
	// set reference
	m_component = &(*coit);

	// search for all hydraulic parameters
	switch (m_component->m_modelType) {
		case HydraulicNetworkComponent::MT_StaticPipe:
		case HydraulicNetworkComponent::MT_DynamicPipe : {
			// retrieve pipe properties
			if(m_pipePropertiesId == INVALID_ID) {
				throw IBK::Exception("Missing pipe property reference!", FUNC_ID);
			}
			// invalid id
			std::vector<HydraulicNetworkPipeProperties>::const_iterator pit =
					std::find(nw.m_pipeProperties.begin(), nw.m_pipeProperties.end(), m_pipePropertiesId);
			if (pit == nw.m_pipeProperties.end()) {
				throw IBK::Exception(IBK::FormatString("Invalid pipe properties reference with id %1.")
									 .arg(m_pipePropertiesId), FUNC_ID);
			}
			// set reference
			m_pipeProperties = &(*pit);

			// all pipes need parameter Length
			m_para[P_Length].checkedValue("Length", "m", "m", 0, false, std::numeric_limits<double>::max(), true,
										   "Length must be > 0 m.");

			// Note: Pipe properties have been checked already.
		}
		break;

		// TODO : add checks for other components
		default: break;
	}


	// check parameters required for thermal balances/heat exchange

	bool heatExchangeDataFileMustExist = false;

	if(nw.m_modelType == HydraulicNetwork::MT_ThermalHydraulicNetwork) {

		// TODO : clarify which parameters are required for which model and check accordingly

		// decide which heat exchange is chosen
		switch(m_component->m_heatExchangeType) {

			case HydraulicNetworkComponent::HT_TemperatureConstant: {
				// check temperature
				m_para[P_Temperature].checkedValue("Temperature", "C", "C", -200.0, true, std::numeric_limits<double>::max(), true,
											   "Temperature must be >= -200 C.");
				// check external heat transfer coefficient
				if(m_component->m_para[HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].name.empty()){
					throw IBK::Exception(IBK::FormatString("Missing parameteter '%1 for exchange type %2!")
								.arg(KeywordList::Keyword("HydraulicNetworkComponent::para_t",
								HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient))
								.arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								m_component->m_heatExchangeType)),
								FUNC_ID);
				}
			} break;

			case HydraulicNetworkComponent::HT_HeatFluxConstant: {
				// check heat flux
				m_para[P_HeatFlux].checkedValue("HeatFlux", "W", "W",
							std::numeric_limits<double>::lowest(), false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature: {

				// check for zone id
				if(m_intPara[IP_ZoneId].name.empty()) {
					throw IBK::Exception(IBK::FormatString("Missing IntParameter '%1 for exchange type %2!")
								.arg(KeywordList::Keyword("HydraulicNetworkElement::intpara_t",
								IP_ZoneId))
								.arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								m_component->m_heatExchangeType)),
								FUNC_ID);

				}
				// check for external heat transfer coefficient
				if(m_component->m_para[HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].name.empty()){
					throw IBK::Exception(IBK::FormatString("Missing parameteter '%1 for exchange type %2!")
								.arg(KeywordList::Keyword("HydraulicNetworkComponent::para_t",
								HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient))
								.arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								m_component->m_heatExchangeType)),
								FUNC_ID);
				}
			} break;

			case HydraulicNetworkComponent::HT_HeatFluxDataFile:
			case HydraulicNetworkComponent::HT_TemperatureDataFile:{
				heatExchangeDataFileMustExist = true;
				break;
			}
			case HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature: {
				throw IBK::Exception(IBK::FormatString("Heat exchange type %1 is not supported, yet!")
							.arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
							m_component->m_heatExchangeType)),
							FUNC_ID);
			}
			case HydraulicNetworkComponent::HT_Adiabatic: {
				// only allowed for pipes
				if(m_component->m_modelType != HydraulicNetworkComponent::MT_StaticPipe &&
				   m_component->m_modelType != HydraulicNetworkComponent::MT_DynamicPipe) {
					throw IBK::Exception(IBK::FormatString("Heat exchange type %1 i^^^^s only allowed for model type 'StaticPipe' "
														   "and 'DynamicPipe'!")
										 .arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
										 m_component->m_heatExchangeType)),
										 FUNC_ID);
				}
			} break;
			case HydraulicNetworkComponent::NUM_HT:
				// No thermal exchange, nothing to initialize
			break;
		}
	}

	// check and read csv file
	if (heatExchangeDataFileMustExist){
		// replace place holders
		m_heatExchangeDataFile = m_heatExchangeDataFile.withReplacedPlaceholders(placeholders);
		if (!m_heatExchangeDataFile.isValid())
			throw IBK::Exception("Missing heat exchange data file path.", FUNC_ID);
		if (!m_heatExchangeDataFile.exists())
			throw IBK::Exception(IBK::FormatString("File '%1' does not exist").arg(m_heatExchangeDataFile.str()), FUNC_ID);
		IBK::CSVReader reader;
		try {
			reader.read(m_heatExchangeDataFile, false, true);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading file '%1'.").arg(m_heatExchangeDataFile.str()), FUNC_ID);
		}

		if (reader.m_nColumns != 2)
			throw IBK::Exception(IBK::FormatString("File '%1' must have exactly 2 columns")
								 .arg(m_heatExchangeDataFile.str()), FUNC_ID);

		if (reader.m_nRows < 2)
			throw IBK::Exception(IBK::FormatString("File '%1' must have at least 2 rows")
								 .arg(m_heatExchangeDataFile.str()), FUNC_ID);

		IBK::Unit xUnit(reader.m_units[0]);
		IBK::Unit yUnit(reader.m_units[1]);

		// create spline
		m_heatExchangeSpline = LinearSplineParameter(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
																			m_component->m_heatExchangeType),
													 LinearSplineParameter::I_LINEAR,
													 reader.colData(0), reader.colData(1), xUnit, yUnit);

		if (m_heatExchangeDataFileIsCyclic)
			m_heatExchangeSpline.m_wrapMethod = LinearSplineParameter::C_CYCLIC;
		else
			m_heatExchangeSpline.m_wrapMethod = LinearSplineParameter::C_CONTINUOUS;

		if (m_component->m_heatExchangeType == HydraulicNetworkComponent::HT_TemperatureDataFile){
			//  check the spline and convert it to base units automatically
			m_heatExchangeSpline.checkAndInitialize("TemperatureDataFile", IBK::Unit("s"), IBK::Unit("K"),
													IBK::Unit("K"), 0, false, std::numeric_limits<double>::max(), false,
													"Temperature must be > 0 K.");
		}

		else if (m_component->m_heatExchangeType == HydraulicNetworkComponent::HT_HeatFluxDataFile){
			//  check the spline and convert it to base units automatically
			m_heatExchangeSpline.checkAndInitialize("HeatFluxDataFile", IBK::Unit("s"), IBK::Unit("W"),
													IBK::Unit("W"), std::numeric_limits<double>::lowest(), false,
													std::numeric_limits<double>::max(), false,
													nullptr);
		}
	}
	else if (m_heatExchangeDataFile.isValid()) {
		throw IBK::Exception(IBK::FormatString("Invalid/unexpected heat exchange data file parameter with value '%1'.")
							 .arg(m_heatExchangeDataFile.str()), FUNC_ID);
	}

}


} // namespace NANDRAD
