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

#ifndef VICUS_ZoneTemplateH
#define VICUS_ZoneTemplateH

#include <IBK_IntPara.h>

#include <QColor>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_InternalLoad.h"
#include "VICUS_Schedule.h"
#include "VICUS_ZoneControlThermostat.h"
#include "VICUS_Infiltration.h"
#include "VICUS_VentilationNatural.h"
#include "VICUS_ZoneIdealHeatingCooling.h"

namespace VICUS {

/*! Describes the course of a *single* ZoneTemplated quantity (basically a value over time data set).
 *  This ZoneTemplate does not have a unit.
*/
class ZoneTemplate : public AbstractDBElement {
public:

	/*! Types used to identify individual sub-templates. */
	enum SubTemplateType {
		//dont change the order
		//first have a look to
		//SVSimulationStartNandrad::generateBuildingProjectData
		ST_IntLoadPerson,					// Keyword: IntLoadPerson
		ST_IntLoadEquipment,				// Keyword: IntLoadEquipment
		ST_IntLoadLighting,					// Keyword: IntLoadLighting
		ST_IntLoadOther,					// Keyword: IntLoadOther
		ST_ControlThermostat,				// Keyword: ControlThermostat
		ST_ControlNaturalVentilation,		// Keyword: ControlNaturalVentilation
		ST_Infiltration,					// Keyword: Infiltration
		ST_VentilationNatural,				// Keyword: NaturalVentilation
		ST_IdealHeatingCooling,				// Keyword: IdealHeatingCooling
		NUM_ST
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	ZoneTemplate();

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced ZoneTemplate is valid. */
	bool isValid(const Database<InternalLoad> & intLoadDB,
				 const Database<ZoneControlThermostat> & thermostatDB,
				 const Database<Schedule> &schedulesDB,
				 const Database<Infiltration> &infiltraionDB,
				 const Database<VentilationNatural> &ventilationDB,
				 const Database<ZoneIdealHeatingCooling> &idealHeatingCoolingDB) const;

	/*! Returns number of assigned sub-templates (needed by tree-model). */
	unsigned int subTemplateCount() const;

	/*! Returns the type of reference by index, counting only the used references, i.e. references not INVALID_ID.
		For example, if m_idReferences[ST_IntLoadPerson] == INVALID_ID and m_idReferences[ST_IntLoadEquipment] has a valid ID, than
		usedReference(0) returns ST_IntLoadEquipment.
		\return Returns type of corresponding id reference or NUM_ST, if index is larger than the number of non-empty
				id references.
	*/
	SubTemplateType usedReference(unsigned int index) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of ZoneTemplate. */
	unsigned int					m_id						= INVALID_ID;		// XML:A:required

	/*! Display name of ZoneTemplate. */
	IBK::MultiLanguageString		m_displayName;									// XML:A

	/*! False color. */
	QColor							m_color;										// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;										// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;									// XML:E

	/*! Stores id references for all sub-templates. */
	IDType							m_idReferences[NUM_ST];							// XML:E
};


} // namespace VICUS


#endif // VICUS_ZoneTemplateH
