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

#ifndef NANDRAD_LocationH
#define NANDRAD_LocationH

#include <IBK_Path.h>
#include <IBK_Flag.h>
#include <IBK_Parameter.h>

#include "NANDRAD_Sensor.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Class Location specifies climate and climatic loads of the
	whole building.
*/
class Location {
public:

	/*! Location parameters. */
	enum para_t {
		/*! Latitude. */
		P_Latitude,					// Keyword: Latitude		[Deg]	'Latitude.'
		/*! Longitude. */
		P_Longitude,				// Keyword: Longitude		[Deg]	'Longitude.'
		/*! Albedo value [0..100 %]. */
		P_Albedo,					// Keyword: Albedo			[%]		'Albedo value [0..100 %].'
		/*! Altitude of building as height above NN [m]. */
		P_Altitude,					// Keyword: Altitude		[m]		'Altitude of building as height above NN [m].'
		NUM_P
	};

	/*! Calculation flags. */
	enum flag_t {
		F_PerezDiffuseRadiationModel,	// Keyword: PerezDiffuseRadiationModel		'Use diffuse radiation model for anisotropic radiation (Perez)'
		F_ContinuousShadingFactorData,	// Keyword: ContinuousShadingFactorData		'If true, shading factors for exterior shading are stored for continuous time points (no cyclic use)'
		NUM_F
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters() const;

	/*! Init default values (called before readXML()).
		\note These values will be overwritten in readXML() when the respective property is set
			  in the project file.
	*/
	void initDefaults();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Parameter set. */
	IBK::Parameter				m_para[NUM_P];							// XML:E
	/*! List of flags. */
	IBK::Flag					m_flags[NUM_F];							// XML:E

	/*! Time zone, only used when both P_Longitude and P_Latitude are given and thus
		a custom climatic location is being used.
	*/
	int							m_timeZone = 13;						// XML:E

	/*! Path to the climate data file (may contain placeholders). */
	IBK::Path					m_climateFilePath;						// XML:E

	/*! Optional: name of the external shading factor data file. */
	IBK::Path					m_shadingFactorFilePath;				// XML:E

	/*! We may place one or more radiation sensors outside. */
	std::vector<Sensor>			m_sensors;								// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_LocationH
