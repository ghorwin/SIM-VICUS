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

#ifndef NANDRAD_HydraulicNetworkH
#define NANDRAD_HydraulicNetworkH

#include <vector>

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"
#include "NANDRAD_HydraulicNetworkControlElement.h"
#include "NANDRAD_HydraulicNetworkNode.h"


namespace NANDRAD {

class Project;

/*! Contains all data for a hydraulic network. */
class HydraulicNetwork {
	NANDRAD_READWRITE_PRIVATE
public:

	/*! The various types (equations) of the hydraulic component. */
	enum ModelType {
		MT_HydraulicNetwork,				// Keyword: HydraulicNetwork				'Only Hydraulic calculation with constant temperature'
		MT_ThermalHydraulicNetwork,			// Keyword: ThermalHydraulicNetwork			'Thermo-hydraulic calculation'
		MT_AirNetwork,						// Keyword: AirNetwork						'Air network that may be connected with zones.'
		NUM_MT
	};

	/*! Parameters for the element . */
	enum para_t {
		P_DefaultFluidTemperature,			// Keyword: DefaultFluidTemperature	[C]		'Default temperature for HydraulicNetwork models'
		P_InitialFluidTemperature,			// Keyword: InitialFluidTemperature	[C]		'Initial temperature of the fluid'
		P_ReferencePressure,				// Keyword: ReferencePressure [Pa]			'Reference pressure of network'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOT_INVALID_ID
	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters(const Project & prj, std::set<unsigned int> &otherNodeIds) ;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this network. */
	unsigned int									m_id			= INVALID_ID;					// XML:A:required
	/*! Descriptive name. */
	std::string										m_displayName;									// XML:A

	ModelType										m_modelType		= MT_ThermalHydraulicNetwork;	// XML:A:required

	/*! At the inlet node of the reference element the reference pressure will be applied
		(usually, this should be a pump model).
	*/
	unsigned int									m_referenceElementId = INVALID_ID;				// XML:A:required

	/*! Fluid properties. */
	HydraulicFluid									m_fluid;										// XML:E:required

	/*! Global network parameters. */
	IBK::Parameter									m_para[NUM_P];									// XML:E

	/*! Pipes used in this network. */
	std::vector<HydraulicNetworkPipeProperties>		m_pipeProperties;								// XML:E

	/*! Hydraulic components used in this network. */
	std::vector<HydraulicNetworkComponent>			m_components;									// XML:E

	/*! List of nodes that are used to connect flow elements. */
	std::vector<HydraulicNetworkNode>				m_nodes;										// XML:E

	/*! List of flow elements that make up this network. */
	std::vector<HydraulicNetworkElement>			m_elements;										// XML:E

	/*! List of mass flow controller elements. */
	std::vector<HydraulicNetworkControlElement>		m_controlElements;								// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkH
