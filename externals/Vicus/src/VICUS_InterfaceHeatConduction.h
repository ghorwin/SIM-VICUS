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

#ifndef VICUS_InterfaceHeatConductionH
#define VICUS_InterfaceHeatConductionH

#include <QString>
#include <QCoreApplication>

#include <vector>

#include <IBK_Parameter.h>
#include <IBK_MultiLanguageString.h>

#include <NANDRAD_InterfaceHeatConduction.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Schedule.h"
#include "VICUS_Database.h"

namespace VICUS {

/*! Enhanced InterfaceHeatConduction definition. */
class InterfaceHeatConduction {
	Q_DECLARE_TR_FUNCTIONS(InterfaceHeatConduction)
public:

	/*! Model types supported by this model. */
	enum modelType_t {
		/*! Constant transfer coefficient. */
		MT_Constant,				// Keyword: Constant		'Constant heat exchange coefficient'
		NUM_MT						// Keyword: None			'No convective heat exchange'
	};

	/*! Defines what kind of zone is connected via this boundary condition.
		If 'Constant' or 'Scheduled' are used when neighbouring zones are not calculated themselves
		but their temperature is known. For ground zones, i.e. in case of soil contact to the construction
		the heat exchange coeff. should be very high 1000 W/m2K to neglect the heat transfer resistance.
	*/
	enum OtherZoneType {
		OZ_Standard,			// Keyword: Standard			'Active zone or outside'
		OZ_Constant,			// Keyword: Constant			'Zone/ground with constant temperature'
		OZ_Scheduled,			// Keyword: Scheduled			'Zone/ground with scheduled temperature'
		NUM_OZ
	};

	/*! Parameters. */
	enum para_t {
		/*! Constant heat transfer coefficient [W/m2K] */
		P_HeatTransferCoefficient,	// Keyword: HeatTransferCoefficient [W/m2K]		'Convective heat transfer coefficient'
		/*! Reference to a const temperature of neighboring zone (required for OZ_Constant). */
		P_ConstTemperature,			// Keyword: ConstTemperature [C]				'Constant temperature of other zone/ground'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMP(InterfaceHeatConduction)

	/*! Checks if all parameters are valid. */
	bool isValid(const Database<Schedule> &scheduleDB) const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;					// XML:A:required

	OtherZoneType						m_otherZoneType = OZ_Standard;			// XML:E

	/*! Reference to a schedule for OZ_Scheduled. */
	IDType								m_idSchedule = INVALID_ID;				// XML:E

	/*! List of constant parameters. */
	IBK::Parameter						m_para[NUM_P];							// XML:E

};

} // namespace VICUS


#endif // VICUS_InterfaceHeatConductionH
