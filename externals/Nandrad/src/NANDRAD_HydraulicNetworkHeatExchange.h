/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NANDRAD_HydraulicNetworkHeatExchangeH
#define NANDRAD_HydraulicNetworkHeatExchangeH

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#include <IBK_IntPara.h>

namespace NANDRAD {

class ConstructionInstance;
class Zone;

/*! Encapsulates all data defining heat exchange between flow elements and
	the environment or other models/elements.

	Definition of heat exchange is done in each flow element definition. If missing, the flow
	element is treated as adiabat.
*/
class HydraulicNetworkHeatExchange {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Defines the type of heat exchange */
	enum ModelType {
		T_TemperatureConstant,				// Keyword: TemperatureConstant			'Constant temperature'
		T_TemperatureSpline,				// Keyword: TemperatureSpline			'Time-dependent temperature from spline'
		T_TemperatureSplineEvaporator,		// Keyword: TemperatureSplineEvaporator	'Evaporator medium temperature for heat pump'
		T_TemperatureZone,					// Keyword: TemperatureZone				'Zone air temperature'
		T_TemperatureConstructionLayer,		// Keyword: TemperatureConstructionLayer 'Active construction layer (floor heating)'
		T_HeatLossConstant,					// Keyword: HeatLossConstant			'Constant heat loss'
		T_HeatLossSpline,					// Keyword: HeatLossSpline				'Heat loss from spline'
		/*! Heat loss from condenser is not the heat loss of the fluid, hence different parameter than T_HeatLossSpline. */
		T_HeatLossSplineCondenser,			// Keyword: HeatLossSplineCondenser		'Heat loss of condenser in heat pump model'
		/*! Heating demand for space heating  */
		T_HeatingDemandSpaceHeating,		// Keyword: HeatingDemandSpaceHeating	'Heating demand for space heating'
		NUM_T
	};

	HydraulicNetworkHeatExchange() {
		for (unsigned int & i : m_idReferences) i = INVALID_ID;
	}

	HydraulicNetworkHeatExchange(ModelType modelType) {
		HydraulicNetworkHeatExchange();
		m_modelType = modelType;
	}

	NANDRAD_READWRITE

	/*! Tests all parameter and initializes linear spline parameters for calculation,
		including reading of potentially referenced TSV files.
		When called from VICUS, keepTsvFileValues should be false, to avoid writing the values into the NANDRAD file.
	*/
	void checkParameters(const std::map<std::string, IBK::Path> &placeholders,
						 const std::vector<Zone> &zones,
						 const std::vector<ConstructionInstance> &conInstances, bool keepTsvFileValues);

	bool operator!=(const HydraulicNetworkHeatExchange & other) const;

	/*! Parameters for the element . */
	enum para_t {
		P_Temperature,						// Keyword: Temperature							[C]		'Temperature for heat exchange'
		P_HeatLoss,							// Keyword: HeatLoss							[W]		'Constant heat flux out of the element (heat loss)'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary'
		NUM_P
	};

	/*! Spline parameter as functions of time. Defined similarly as time series for location object (i.e. with start time shift). */
	enum splinePara_t {
		SPL_Temperature,					// Keyword: Temperature							[C]		'Temperature for heat exchange'
		/*! Heat loss spline is used for models T_HeatLossSpline and T_HeatLossSplineCondenser. */
		SPL_HeatLoss,						// Keyword: HeatLoss							[W]		'Constant heat flux out of the element (heat loss)'
		NUM_SPL
	};

	/*! Integer/whole number parameters. */
	enum References {
		ID_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
		ID_ConstructionInstanceId,			// Keyword: ConstructionInstanceId				[-]		'ID of coupled construction instance for thermal exchange'
		NUM_ID
	};

	/*! Model/type of heat exchange. */
	ModelType						m_modelType = NUM_T;									// XML:A

	/*! Integer parameters. */
	IDType							m_idReferences[NUM_ID];									// XML:E

	/*! Parameter */
	IBK::Parameter					m_para[NUM_P];											// XML:E

	/*! Time-series of heat flux or temperature (can be spline or tsv-file). */
	LinearSplineParameter			m_splPara[NUM_SPL];										// XML:E


	// *** Static functions ***

	static std::vector<ModelType> availableHeatExchangeTypes(const HydraulicNetworkComponent::ModelType modelType);
};


} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkHeatExchangeH
