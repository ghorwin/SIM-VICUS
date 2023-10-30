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

#ifndef VICUS_EmbeddedDatabaseH
#define VICUS_EmbeddedDatabaseH

#include "VICUS_CodeGenMacros.h"

#include "VICUS_Material.h"
#include "VICUS_Component.h"
#include "VICUS_Construction.h"
#include "VICUS_SubSurfaceComponent.h"
#include "VICUS_WindowGlazingSystem.h"
#include "VICUS_Window.h"
#include "VICUS_WindowFrame.h"
#include "VICUS_WindowDivider.h"
#include "VICUS_AcousticBoundaryCondition.h"
#include "VICUS_BoundaryCondition.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkComponent.h"
#include "VICUS_NetworkController.h"
#include "VICUS_SubNetwork.h"
#include "VICUS_SupplySystem.h"
#include "VICUS_EpdDataset.h"
#include "VICUS_Schedule.h"
#include "VICUS_InternalLoad.h"
#include "VICUS_ZoneControlShading.h"
#include "VICUS_ZoneControlNaturalVentilation.h"
#include "VICUS_ZoneTemplate.h"
#include "VICUS_SurfaceHeating.h"
#include "VICUS_AcousticSoundAbsorption.h"

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

	/*! Map of all database glazing systems. */
	std::vector<VICUS::WindowGlazingSystem>				m_windowGlazingSystems;				// XML:E

	/*! Map pf all database acoustic boundary conditions */
	std::vector<VICUS::AcousticBoundaryCondition>		m_acousticBoundaryConditions;		// XML:E

	/*! Map pf all database acoustic boundary conditions */
	std::vector<VICUS::AcousticSoundAbsorption>					m_acousticSoundAbsorptions;		// XML:E

	/*! Map of all database boundary conditions. */
	std::vector<VICUS::BoundaryCondition>				m_boundaryConditions;				// XML:E

	/*! Map of all database components. */
	std::vector<VICUS::Component>						m_components;						// XML:E

	/*! Map of all database subsurface components (holes/windows...). */
	std::vector<VICUS::SubSurfaceComponent>				m_subSurfaceComponents;				// XML:E

	/*! Map of all database subsurface components (holes/windows...). */
	std::vector<VICUS::SurfaceHeating>					m_surfaceHeatings;					// XML:E

	/*! Map of all database supply systems. */
	std::vector<VICUS::SupplySystem>					m_supplySystems;				// XML:E

	/*! Map of all database pipes */
	std::vector<VICUS::NetworkPipe>						m_pipes;							// XML:E

	/*! Map of all database fluids */
	std::vector<VICUS::NetworkFluid>					m_fluids;							// XML:E

	/*! Map of all hydraulic network components */
	std::vector<VICUS::NetworkComponent>				m_networkComponents;				// XML:E

	/*! Map of all hydraulic network controllers */
	std::vector<VICUS::NetworkController>				m_networkControllers;				// XML:E

	/*! Map of all hydraulic sub networks */
	std::vector<VICUS::SubNetwork>						m_subNetworks;						// XML:E

	/*! Map of all database EPD elements */
	std::vector<VICUS::EpdDataset>						m_EPDDatasets;						// XML:E

	/*! Map of all database schedules */
	std::vector<VICUS::Schedule>						m_schedules;						// XML:E

	/*! Map of all database Internal loads. */
	std::vector<VICUS::InternalLoad>					m_internalLoads;					// XML:E

	/*! Map of all database Zone Control Thermostats. */
	std::vector<VICUS::ZoneControlThermostat>			m_zoneControlThermostats;			// XML:E

	/*! Map of all database Zone Control Thermostats. */
	std::vector<VICUS::ZoneControlShading>				m_zoneControlShading;				// XML:E

	/*! Map of all database Zone Control Thermostats. */
	std::vector<VICUS::ZoneIdealHeatingCooling>			m_zoneIdealHeatingCooling;			// XML:E

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
