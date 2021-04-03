#ifndef VICUS_EmbeddedDatabaseH
#define VICUS_EmbeddedDatabaseH

#include "VICUS_CodeGenMacros.h"

#include "VICUS_Material.h"
#include "VICUS_Component.h"
#include "VICUS_Construction.h"
#include "VICUS_WindowGlazingSystem.h"
#include "VICUS_Window.h"
#include "VICUS_WindowFrame.h"
#include "VICUS_WindowDivider.h"
#include "VICUS_SurfaceProperties.h"
#include "VICUS_BoundaryCondition.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkComponent.h"
#include "VICUS_EPDDataset.h"
#include "VICUS_Schedule.h"
#include "VICUS_InternalLoad.h"
#include "VICUS_ZoneControlThermostat.h"
#include "VICUS_ZoneControlShading.h"
#include "VICUS_ZoneControlNaturalVentilation.h"
#include "VICUS_Infiltration.h"
#include "VICUS_VentilationNatural.h"
#include "VICUS_ZoneTemplate.h"

namespace VICUS {

/*! This stores the database elements used in a project and are needed for
	transfering project data (with user-defined database elements) from one
	installation to another, and to generate NANDRAD projects.
*/
class EmbeddedDatabase {
public:

	VICUS_READWRITE

	/*! Map of all opaque database materials. */
	std::vector<VICUS::Material>						m_materials;						// XML:E

	/*! Map of all database constructions. */
	std::vector<VICUS::Construction>					m_constructions;					// XML:E

	/*! Map of all window definitions. */
	std::vector<VICUS::Window>							m_windows;							// XML:E

//	/*! Map of all database glazing systems. */
//	std::vector<VICUS::WindowGlazingSystem>			m_windowGlazingSystems;

	/*! Map of all database boundary conditions. */
	std::vector<VICUS::BoundaryCondition>				m_boundaryConditions;				// XML:E

	/*! Map of all database components. */
	std::vector<VICUS::Component>						m_components;						// XML:E

	/*! Map of all database pipes */
	std::vector<VICUS::NetworkPipe>						m_pipes;							// XML:E

	/*! Map of all database fluids */
	std::vector<VICUS::NetworkFluid>					m_fluids;							// XML:E

	/*! Map of all hydraulic network components */
	std::vector<VICUS::NetworkComponent>				m_networkComponents;				// XML:E

	/*! Map of all database EPD elements */
	std::vector<VICUS::EPDDataset>						m_EPDElements;						// XML:E

	/*! Map of all database schedules */
	std::vector<VICUS::Schedule>						m_schedules;						// XML:E

	/*! Map of all database Internal loads. */
	std::vector<VICUS::InternalLoad>					m_internalLoads;					// XML:E

	/*! Map of all database Zone Control Thermostats. */
	std::vector<VICUS::ZoneControlThermostat>			m_zoneControlThermostat;			// XML:E

	/*! Map of all database Zone Control Thermostats. */
	std::vector<VICUS::ZoneControlShading>				m_zoneControlShading;				// XML:E

	/*! Map of all database Zone Control Natural Ventilation. */
	std::vector<VICUS::ZoneControlNaturalVentilation>	m_zoneControlVentilationNatural;	// XML:E

	/*! Map of all database Natural Ventilation. */
	std::vector<VICUS::VentilationNatural>				m_ventilationNatural;				// XML:E

	/*! Map of all database Infiltration. */
	std::vector<VICUS::Infiltration>					m_infiltration;						// XML:E

	/*! Map of all database Zone templates. */
	std::vector<VICUS::ZoneTemplate>					m_zoneTemplates;					// XML:E

};

} // namespace VICUS


#endif // VICUS_EmbeddedDatabaseH
