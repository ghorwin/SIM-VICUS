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

#ifndef NANDRAD_HydraulicNetworkPipePropertiesH
#define NANDRAD_HydraulicNetworkPipePropertiesH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*! Stores data specific for a pipe. */
class HydraulicNetworkPipeProperties {
public:

	/*! Parameters for the component. */
	enum para_t {
		P_PipeRoughness,					// Keyword: PipeRoughness						[mm]		'Roughness of pipe material'
		P_PipeInnerDiameter,				// Keyword: PipeInnerDiameter					[mm]		'Inner diameter of pipe'
		P_PipeOuterDiameter,				// Keyword: PipeOuterDiameter					[mm]		'Outer diameter of pipe'
		P_UValueWall,						// Keyword: UValueWall							[W/mK]		'Length-specific U-Value of pipe wall incl. insulation'
		P_HeatCapacityWall,					// Keyword: HeatCapacityWall					[J/kgK]		'Specific heat capaciy of pipe wall'
		P_DensityWall,						// Keyword: DensityWall							[kg/m3]		'Density of pipe wall'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(HydraulicNetworkPipeProperties)

	/*! Checks for valid and required parameters (value ranges).
		\param networkModelType Identifies basic network model type (see HydraulicNetwork::ModelType)
	*/
	void checkParameters(int networkModelType) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this component. */
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required

	/*! Parameters. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkPipePropertiesH
