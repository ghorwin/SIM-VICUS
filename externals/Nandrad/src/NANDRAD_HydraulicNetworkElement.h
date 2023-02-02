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

#ifndef NANDRAD_HydraulicNetworkElementH
#define NANDRAD_HydraulicNetworkElementH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_HydraulicNetworkHeatExchange.h"
#include "NANDRAD_IDVectorMap.h"


namespace NANDRAD {

class HydraulicNetworkComponent;
class HydraulicNetworkPipeProperties;
class HydraulicNetwork;
class HydraulicNetworkControlElement;
class Project;

/*! Contains data of a flow element inside a network.
	Pipe-specific parameters are stored in HydraulicNetworkPipeProperties objects, referenced via m_pipeID. For
	other elements, this ID is unused.
*/
class HydraulicNetworkElement {
public:

	HydraulicNetworkElement() 	{
		for(IBK::IntPara &i : m_intPara)
			i.value = (int) NANDRAD::INVALID_ID;
	}

	/*! C'tor for pipes. */
	HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
							unsigned int inletZoneId, unsigned int outletZoneId,
							unsigned int componentId, unsigned int pipeID, double length);

	/*! C'tor for a network element other than pipes. */
	HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
							unsigned int inletZoneId, unsigned int outletZoneId,
							unsigned int componentId, unsigned int controlElementId);

	/*! Parameters for the element . */
	enum para_t {
		P_Length,						// Keyword: Length						[m]		'Pipe length'
		NUM_P
	};

	/*! Whole number parameters. */
	enum intPara_t {
		IP_NumberParallelPipes,			// Keyword: NumberParallelPipes					'Number of parallel pipes'
		IP_NumberParallelElements,		// Keyword: NumberParallelElements				'Number of parallel elements'
		NUM_IP
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	bool operator!=(const HydraulicNetworkElement &other) const;

	/*! Checks for valid and required parameters (value ranges).
		If referenced TSV file is given, this is read and checked and data is transferred into
		variable m_tsvFileContent.
	*/
	void checkParameters(const HydraulicNetwork & nw, const Project & prj);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this flow component.
		ID is used for outputs and to reference heat sources/sinks connected to this element. For active elements,
		this ID is used to connect control models.
	*/
	IDType							m_id				= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Inlet node ID. */
	IDType							m_inletNodeId		= NANDRAD::INVALID_ID;				// XML:A
	/*! Outlet node ID. */
	IDType							m_outletNodeId		= NANDRAD::INVALID_ID;				// XML:A
	/*! Inlet node ID. */
	IDType							m_inletZoneId		= NANDRAD::INVALID_ID;				// XML:A
	/*! Outlet node ID. */
	IDType							m_outletZoneId		= NANDRAD::INVALID_ID;				// XML:A
	/*! Hydraulic component ID. */
	IDType							m_componentId		= NANDRAD::INVALID_ID;				// XML:A
	/*! Pipe ID (only needed for elements that are pipes). */
	IDType							m_pipePropertiesId	= NANDRAD::INVALID_ID;				// XML:A

	/*! Optional reference to a flow controller element. */
	IDType							m_controlElementId	= NANDRAD::INVALID_ID;				// XML:A

	/*! Display name. */
	std::string						m_displayName;											// XML:A

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];											// XML:E

	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];										// XML:E

	/*! Optional definition of heat exchange calculation model (if missing, flow element is adiabat). */
	HydraulicNetworkHeatExchange	m_heatExchange;											// XML:E

	/*! Stores ids of hydraulic network elements that shall be observed for worst-point controller
	 *  (only used if controlElement has option CP_PressureDifferenceWorstpoint) */
	IDVectorMap<unsigned int>		m_observedPressureDiffElementIds;				// XML:E


	// *** Variables used only during simulation ***

	const HydraulicNetworkComponent			*m_component				= nullptr;
	const HydraulicNetworkPipeProperties	*m_pipeProperties			= nullptr;
	const HydraulicNetworkControlElement	*m_controlElement			= nullptr ;

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkElementH
