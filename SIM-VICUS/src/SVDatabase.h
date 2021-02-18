#ifndef SVDatabaseH
#define SVDatabaseH

#include <VICUS_Material.h>
#include <VICUS_Component.h>
#include <VICUS_Construction.h>
#include <VICUS_WindowGlazingSystem.h>
#include <VICUS_Window.h>
#include <VICUS_WindowFrame.h>
#include <VICUS_WindowDivider.h>
#include <VICUS_SurfaceProperties.h>
#include <VICUS_BoundaryCondition.h>
#include <VICUS_NetworkPipe.h>
#include <VICUS_NetworkFluid.h>
#include <VICUS_NetworkComponent.h>
#include <VICUS_EPDDataset.h>
#include <VICUS_Schedule.h>

#include <VICUS_Database.h>

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
		DT_Components,
		DT_BoundaryConditions,
		DT_Windows,
		DT_Pipes,
		DT_Fluids,
		DT_NetworkComponents,
		DT_Schedules,
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
	void writeDatabases() const;


	// Databases

	/*! Map of all opaque database materials. */
	VICUS::Database<VICUS::Material>					m_materials;

	/*! Map of all database constructions. */
	VICUS::Database<VICUS::Construction>				m_constructions;

	/*! Map of all window definitions. */
	VICUS::Database<VICUS::Window>						m_windows;

//	/*! Map of all database glazing systems. */
//	VICUS::Database<VICUS::WindowGlazingSystem>			m_windowGlazingSystems;

	/*! Map of all database boundary conditions. */
	VICUS::Database<VICUS::BoundaryCondition>			m_boundaryConditions;

	/*! Map of all database components. */
	VICUS::Database<VICUS::Component>					m_components;

	/*! Map of all database pipes */
	VICUS::Database<VICUS::NetworkPipe>					m_pipes;

	/*! Map of all database fluids */
	VICUS::Database<VICUS::NetworkFluid>				m_fluids;

	/*! Map of all hydraulic network components */
	VICUS::Database<VICUS::NetworkComponent>			m_networkComponents;

	/*! Map of all database EPD elements */
	VICUS::Database<VICUS::EPDDataset>					m_EPDElements;

	/*! Map of all database schedules */
	VICUS::Database<VICUS::Schedule>					m_schedules;

private:
	/*! Flag signaling whether any item in the database was modified or not. */
	bool	m_modified;
};


#endif // SVDatabaseH
