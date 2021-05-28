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

#ifndef NANDRAD_ProjectH
#define NANDRAD_ProjectH

#include <string>
#include <vector>
#include <map>

#include "NANDRAD_Zone.h"
#include "NANDRAD_ConstructionInstance.h"
#include "NANDRAD_ProjectInfo.h"
#include "NANDRAD_SimulationParameter.h"
#include "NANDRAD_SolverParameter.h"
#include "NANDRAD_Schedules.h"
#include "NANDRAD_Outputs.h"
#include "NANDRAD_Location.h"
#include "NANDRAD_ConstructionType.h"
#include "NANDRAD_Material.h"
#include "NANDRAD_ObjectList.h"
#include "NANDRAD_Models.h"
#include "NANDRAD_WindowGlazingSystem.h"
#include "NANDRAD_HydraulicNetwork.h"
#include "NANDRAD_HydraulicNetworkComponent.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_FMIDescription.h"

/*! The namespace NANDRAD contains the data model classes that make up
	the NANDRAD solver input data. The main class is NANDRAD::Project.
*/
namespace NANDRAD {

/*! Contains all input data that describes a room with walls, floor, ceiling, usage, HVAC etc.
	The room description references constructions, window and shading types from the database.
*/
class Project {
	NANDRAD_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Reads the project data from an XML file.
		\param filename  The full path to the project file.
	*/
	void readXML(const IBK::Path & filename);

	/*! Writes the project file to an XML file.
		\param filename  The full path to the project file.
	*/
	void writeXML(const IBK::Path & filename) const;

	/*! Initializes project defaults.
		This function is called during solver run before readXML(). Hence, default values can be overwritten during
		project file reading.
	*/
	void initDefaults();

	/*! All constructions with same boundary conditions and construction type are merged into one
		since the results will be the same for all constructions.
		For each merged construction, a new construction is added to the end of the construction list.
		All merged constructions are removed (this ensured, that any remaining reference to a merged
		construction will trigger an error).
		The cross sections of all constructions are summed together to make up the final cross section of
		the merged construction.

		A data table is created with information on merged constructions:
		- new merged construction ID
		- vector of merged construction IDs and their areas

		\warning This algorithmus may not be suitable for calculations that involve view factors and
		long wave radiation exchange. Hence, do not call this function for such models!

		Embedded objects that reference one of the merge constructions will be modified to reference
		the merge construction instead. A mapping table is created that lists those changes:
		- embedded object ID
		- old construction instance ID
		- new merged construction instance ID
	*/
	void mergeSameConstructions();



	/*! Comments about the project. */
	ProjectInfo										m_projectInfo;						// XML:E

	/*! Contains mapping of directory placeholders and absolute directory paths. */
	std::map<std::string, IBK::Path>				m_placeholders;

	/*! Location of the building and climate data. */
	Location										m_location;							// XML:E

	/*! Simulation settings: all global parameters.*/
	SimulationParameter								m_simulationParameter;				// XML:E

	/*! Solver settings: error tolerances and convergence coefficients*/
	SolverParameter									m_solverParameter;					// XML:E

	/*! All active and constant thermal zones.*/
	std::vector<Zone>								m_zones;							// XML:E

	/*! All construction instances refernce a construction and a thermal zone. */
	std::vector<ConstructionInstance>				m_constructionInstances;			// XML:E

	/*! All hydraulic networks defined for this project. */
	std::vector<HydraulicNetwork>					m_hydraulicNetworks;				// XML:E

	/*! All construction types reference construction parameters. */
	std::vector<ConstructionType>					m_constructionTypes;				// XML:E

	/*! All material types. */
	std::vector<Material>							m_materials;						// XML:E

	/*! All glazing types. */
	std::vector<WindowGlazingSystem>				m_windowGlazingSystems;				// XML:E

	/*! References to all schedules.*/
	Schedules										m_schedules;						// XML:E

	/*! Container for various model parametrization blocks. */
	Models											m_models;							// XML:E

	/*! References to Output specifications.*/
	Outputs											m_outputs;							// XML:E

	/*! References to object lists.*/
	std::vector<ObjectList>							m_objectLists;						// XML:E

	/*! Definitions for exporting an FMU from the model. */
	FMIDescription									m_fmiDescription;					// XML:E

private:

	/*! Reads the section with directory placeholders.
		\param element The directory placeholders element tag.
	*/
	void readDirectoryPlaceholdersXML(const TiXmlElement * element);
	/*! Writes the section with directory placeholders, but only, if the map isn't empty.
		\param parent The parent tag.
	*/
	void writeDirectoryPlaceholdersXML(TiXmlElement * parent) const;

};


} // namespace NANDRAD

#endif // NANDRAD_ProjectH
