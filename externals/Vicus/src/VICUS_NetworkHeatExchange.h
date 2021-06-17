/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#ifndef VICUS_NetworkHeatExchangeH
#define VICUS_NetworkHeatExchangeH

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_HydraulicNetworkHeatExchange.h"

namespace VICUS {

/*! Contains all parameters that describe heat exchange between hydraulic network components and anything else.
*/
class NetworkHeatExchange {
public:

//	VICUS_READWRITE

//	/*! Defines the type of heat exchange */
//	enum ModelType {
//		T_TemperatureConstant,				// Keyword: TemperatureConstant			'Difference to constant temperature'
//		T_TemperatureSpline,				// Keyword: TemperatureSpline			'Difference to time-dependent temperature from spline'
//		T_TemperatureSplineEvaporator,		// Keyword: TemperatureSplineEvaporator	'Evaporator medium temperature for heat pump'
//		T_TemperatureZone,					// Keyword: TemperatureZone				'Difference to zone air temperature'
//		T_TemperatureConstructionLayer,		// Keyword: TemperatureConstructionLayer 'Difference to active construction layer (floor heating)'
//		T_HeatLossConstant,					// Keyword: HeatLossConstant			'Constant heat loss'
//		T_HeatLossSpline,					// Keyword: HeatLossSpline				'Heat loss from spline'
//		/*! Heat loss from condenser is not the heat loss of the fluid, hence different parameter than T_HeatLossSpline. */
//		T_HeatLossSplineCondenser,			// Keyword: HeatLossSplineCondenser		'Heat loss of condenser in heat pump model'
//		NUM_T
//	};

//	/*! Parameters for the element . */
//	enum para_t {
//		P_Temperature,						// Keyword: Temperature							[C]		'Temperature for heat exchange'
//		P_HeatLoss,							// Keyword: HeatLoss							[W]		'Constant heat flux out of the element (heat loss)'
//		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary'
//		NUM_P
//	};

//	/*! Spline parameter as functions of time. Defined similarly as time series for location object (i.e. with start time shift). */
//	enum splinePara_t {
//		SPL_Temperature,					// Keyword: Temperature							[C]		'Temperature for heat exchange'
//		/*! Heat loss spline is used for models T_HeatLossSpline and T_HeatLossSplineCondenser. */
//		SPL_HeatLoss,						// Keyword: HeatLoss							[W]		'Constant heat flux out of the element (heat loss)'
//		NUM_SPL
//	};

//	/*! Integer/whole number parameters. */
//	enum References {
//		ID_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
//		ID_ConstructionInstanceId,			// Keyword: ConstructionInstanceId				[-]		'ID of coupled construction instance for thermal exchange'
//		NUM_ID
//	};

//	NetworkHeatExchange(){
//		for (unsigned int & i : m_idReferences) i = INVALID_ID;
//	}

//	NetworkHeatExchange(const ModelType &modelType):
//	m_modelType(modelType){
//		for (unsigned int & i : m_idReferences) i = INVALID_ID;
//	}

//	NANDRAD::HydraulicNetworkHeatExchange toNandradHeatExchange() const;

//	bool operator!=(const NetworkHeatExchange &other) const;

//	ModelType						m_modelType	= NUM_T;					// XML:E

//	IBK::Parameter					m_para[NUM_P];							// XML:E

//	/*! Integer parameters. */
//	IDType							m_idReferences[NUM_ID];					// XML:E

//	/*! Time-series of heat flux or temperature (can be spline or tsv-file).
//		Note: the XML tag name is always the same "HeatExchangeSpline", yet the content (and physical units)
//		differ depending on selected heat exchange type.
//	*/
//	NANDRAD::LinearSplineParameter	m_splPara[NUM_SPL];						// XML:E

};

} // namespace VICUS

#endif // VICUS_NetworkHeatExchangeH
