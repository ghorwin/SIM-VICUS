/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef VICUS_ZoneTemplateH
#define VICUS_ZoneTemplateH

#include <IBK_IntPara.h>

#include <QColor>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

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
		ST_IntLoadPerson,			// Keyword: IntLoadPerson
		ST_IntLoadEquipment,		// Keyword: IntLoadEquipment
		ST_IntLoadLighting,			// Keyword: IntLoadLighting
		ST_IntLoadOther,			// Keyword: IntLoadOther
		ST_ControlThermostat,		// Keyword: ControlThermostat
		ST_Infiltration,			// Keyword: Infiltration
		ST_VentilationNatural,		// Keyword: NaturalVentilation
		NUM_ST
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	ZoneTemplate();

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced ZoneTemplate is valid. */
	bool isValid() const;

	/*! Returns number of assigned sub-templates (needed by tree-model). */
	unsigned int subTemplateCount() const;

	///TODO Dirk->Andreas das funktioniert nicht wie beschrieben oder ich hab die Beschreibung nicht kapiert
	/// wenn ich eine valide id in m_idReferences[ST_IntLoadEquipment] habe z.B. 70001
	/// und sonst invalide ids nur
	/// und ich den index ST_IntLoadEquipment (index wäre dann 1) abfrage
	/// dann liefert er mir NUM_ST
	/// das ist doch falsch oder?
	/*! Returns the type of reference by index, counting only the used references, i.e. references not INVALID_ID.
		For example, if m_idIntLoadPerson == INVALID_ID and m_idIntLoadElectricEquipment has a valid ID, than
		usedReference(0) returns ST_IntLoadEquipment.
		\return Returns type of corresponding id reference or NUM_ST, if index is larger than the number of non-empty
				id references.
	*/
	SubTemplateType usedReference(unsigned int index) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

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

#if 0
	/*! Internal loads person model id. */
	unsigned int					m_idIntLoadPerson			= INVALID_ID;

	/*! Internal loads electric equipment model id. */
	unsigned int					m_idIntLoadElectricEquipment = INVALID_ID;

	/*! Internal loads electric lighting model id. */
	unsigned int					m_idIntLoadLighting			= INVALID_ID;

	/*! Internal loads other equipment model id. */
	unsigned int					m_idIntLoadOther			= INVALID_ID;

	/*! Control thermostat model id. */
	unsigned int					m_idControlThermostat		= INVALID_ID;

	/*! Control shading model id. */
	unsigned int					m_idControlShading			= INVALID_ID;

	/*! Natural ventilation model id. */
	unsigned int					m_idNaturalVentilation		= INVALID_ID;

	/*! Mechanical ventilation model id. */
	unsigned int					m_idMechanicalVentilation	= INVALID_ID;

	/*! Infiltration model id. */
	unsigned int					m_idInfiltration			= INVALID_ID;
#endif
};


} // namespace VICUS


#endif // VICUS_ZoneTemplateH
