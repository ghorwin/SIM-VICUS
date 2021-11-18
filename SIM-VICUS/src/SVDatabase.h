/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVDatabaseH
#define SVDatabaseH

#include <VICUS_Project.h>

/*! Central provider of predefined and user defined construction, window, material... databases.

	Initialize the database once in your program using the init() function and use it
	at different places.

	The items in the lists of the db are uniquely identified through their IDs. The display name
	is usually given as encoded multi-language string.
*/
class SVDatabase {
public:
	enum DatabaseTypes {
		DT_Materials,
		DT_Constructions,
		DT_Windows,
		DT_WindowGlazingSystems,
		DT_BoundaryConditions,
		DT_Components,
		DT_SubSurfaceComponents,
		DT_SurfaceHeating,
		DT_Pipes,
		DT_Fluids,
		DT_NetworkComponents,
		DT_NetworkControllers,
		DT_SubNetworks,
		DT_Schedules,
		DT_InternalLoads,
		DT_ZoneControlThermostat,
		DT_ZoneControlShading,
		DT_ZoneControlNaturalVentilation,
		DT_ZoneIdealHeatingCooling,
		DT_VentilationNatural,
		DT_Infiltration,
		DT_ZoneTemplates,
		NUM_DT // used for "all"
	};

	/*! Standard constructor.*/
	SVDatabase();

	/*! Reads built-in and user-defined database.
		If t is not NUM_DT, only the *user-db* for the selected database is read. Use this to restore the
		user-defined database elements ("Undo" for database editing).
	*/
	void readDatabases(DatabaseTypes t = NUM_DT);

	/*! Writes user-defined database. */
	void writeDatabases();


	/*! Searches through project and all copies all references database elements into
		the embedded project database.
		\warning This replaces any previously stored embedded database elements in the project and only
			uses those stored in this SVDatabase object!
	*/
	void updateEmbeddedDatabase(VICUS::Project & p);

	/*! Holds ids of possible duplicates and whether they are identical. */
	struct DuplicateInfo {
		unsigned int m_idFirst;
		unsigned int m_idSecond;
		bool m_identical;
	};

	/*! Processes all database lists and checks for duplicates. If a duplicate is found (i.e. two database elements
		are either identical except for ID, or only differ in meta-data), their IDs are added to the respective
		vector. Each database element ID appears only once, even if there are three or more identical elements.
		\param duplicatePairs Vector of size NUM_DT with a vector of potential duplicates for each DB element type. Use
							  DatabaseTypes enumeration values as index.
	*/
	void determineDuplicates(std::vector< std::vector<DuplicateInfo> > & duplicatePairs) const;

	/*! Removes element from Database.
		\param dbType The database to remove the element from.
		\param elementID The ID of the element to be removed.
		\param replacementElementID ID to be used instead of removed elementID in all references to the removed DB element.
	*/
	void removeDBElement(DatabaseTypes dbType, unsigned int elementID, unsigned int replacementElementID);

	/*! Convenience function, can be used to lookup a subtype template by argument.
		ID of subtemplate is taken from idReferenceArray at index given by 'st'.
		Returns nullptr if element doesn't exist.
	*/
	const VICUS::AbstractDBElement * lookupSubTemplate(VICUS::ZoneTemplate::SubTemplateType st, const IDType idReferenceArray[]) const;

	// Databases

	/*! Map of all opaque database materials. */
	VICUS::Database<VICUS::Material>					m_materials;

	/*! Map of all database constructions. */
	VICUS::Database<VICUS::Construction>				m_constructions;

	/*! Map of all window definitions. */
	VICUS::Database<VICUS::Window>						m_windows;

	/*! Map of all database glazing systems. */
	VICUS::Database<VICUS::WindowGlazingSystem>			m_windowGlazingSystems;

	/*! Map of all database boundary conditions. */
	VICUS::Database<VICUS::BoundaryCondition>			m_boundaryConditions;

	/*! Map of all database components. */
	VICUS::Database<VICUS::Component>					m_components;

	/*! Map of all database subsurface-components. */
	VICUS::Database<VICUS::SubSurfaceComponent>			m_subSurfaceComponents;

	/*! Map of all database surface heating elements. */
	VICUS::Database<VICUS::SurfaceHeating>				m_surfaceHeatings;

	/*! Map of all database pipes */
	VICUS::Database<VICUS::NetworkPipe>					m_pipes;

	/*! Map of all database fluids */
	VICUS::Database<VICUS::NetworkFluid>				m_fluids;

	/*! Map of all hydraulic network components */
	VICUS::Database<VICUS::NetworkComponent>			m_networkComponents;

	/*! Map of all hydraulic network controllers */
	VICUS::Database<VICUS::NetworkController>			m_networkControllers;

	/*! Map of all hydraulic sub networks */
	VICUS::Database<VICUS::SubNetwork>					m_subNetworks;

	/*! Map of all database EPD elements */
//	VICUS::Database<VICUS::EPDDataset>					m_EPDElements;

	/*! Map of all database schedules */
	VICUS::Database<VICUS::Schedule>					m_schedules;

	/*! Map of all database Internal loads. */
	VICUS::Database<VICUS::InternalLoad>				m_internalLoads;

	/*! Map of all database Zone Control Thermostats. */
	VICUS::Database<VICUS::ZoneControlThermostat>		m_zoneControlThermostat;

	/*! Map of all database Zone Control Thermostats. */
	VICUS::Database<VICUS::ZoneControlShading>			m_zoneControlShading;

	/*! Map of all database Zone Control Natural Ventilation. */
	VICUS::Database<VICUS::ZoneControlNaturalVentilation>	m_zoneControlVentilationNatural;

	/*! Map of all database Zone Ideal Heating and Cooling. */
	VICUS::Database<VICUS::ZoneIdealHeatingCooling>		m_zoneIdealHeatingCooling;

	/*! Map of all database Natural Ventilation. */
	VICUS::Database<VICUS::VentilationNatural>			m_ventilationNatural;

	/*! Map of all database Infiltration. */
	VICUS::Database<VICUS::Infiltration>				m_infiltration;

	/*! Map of all database Zone templates. */
	VICUS::Database<VICUS::ZoneTemplate>				m_zoneTemplates;
};


#endif // SVDatabaseH
