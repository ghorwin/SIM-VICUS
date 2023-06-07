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

#ifndef NANDRAD_SolarLoadsDistributionModelH
#define NANDRAD_SolarLoadsDistributionModelH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! This model stores global parameters related to solar loads distribution in zones.
*/
class SolarLoadsDistributionModel {
	NANDRAD_READWRITE_PRIVATE
public:
	/*! Short wave radiation model variant. */
	enum distribution_t {
		/*! Short wave radiation on surfaces is distributed area-weighted. */
		SWR_AreaWeighted,							// Keyword: AreaWeighted								'Distribution based on surface area'
		/*! Short wave radiation on surface is distributed based on surface type (see P_RadiationLoadFractionFloor, P_RadiationLoadFractionCeiling and P_RadiationLoadFractionWalls). */
		SWR_SurfaceTypeFactor,						// Keyword: SurfaceTypeFactor							'Distribution based on surface type'
		/*! Short wave radiation on surface is distributed via view factor table (defined in each zone). */
		SWR_ViewFactor,								// Keyword: ViewFactor									'Distribution based on zone-specific view factors'
		NUM_SWR
	};

	/*! Parameter values. */
	enum para_t {
		P_RadiationLoadFractionZone,				// Keyword: RadiationLoadFractionZone			[%]		'Percentage of solar radiation gains attributed direcly to room [%]'
		P_RadiationLoadFractionFloor,				// Keyword: RadiationLoadFractionFloor			[%]		'Percentage of surface solar radiation attributed to floor [%]'
		P_RadiationLoadFractionCeiling,				// Keyword: RadiationLoadFractionCeiling		[%]		'Percentage of surface solar radiation attributed to roof/ceiling[%]'
		P_RadiationLoadFractionWalls,				// Keyword: RadiationLoadFractionWalls			[%]		'Percentage of surface solar radiation attributed to walls [%]'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Init default values (called before readXML() in solver).
		\note These values will be overwritten in readXML() when the respective property is set
			  in the project file.
	*/
	void initDefaults();

	NANDRAD_READWRITE_IFNOTEMPTY(SolarLoadsDistributionModel)
	NANDRAD_COMP(SolarLoadsDistributionModel)

	/*! To be called after readXML() and mainly used to check whether user-provided parameters are in the valid ranges. */
	void checkParameters() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Defines the way short wave radiation loads are distributed. */
	distribution_t	m_distributionType = SWR_SurfaceTypeFactor;			// XML:E

	/*! List of parameters. */
	IBK::Parameter		m_para[NUM_P];									// XML:E


};

} // namespace NANDRAD

#endif // NANDRAD_SolarLoadsDistributionModelH
