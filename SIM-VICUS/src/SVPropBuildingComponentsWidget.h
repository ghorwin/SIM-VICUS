#ifndef SVPropBuildingComponentsWidgetH
#define SVPropBuildingComponentsWidgetH

#include <QWidget>

#include <VICUS_Constants.h>

namespace Ui {
	class SVPropBuildingComponentsWidget;
}

namespace VICUS {
	class Surface;
	class Component;
}

/*! The property page showing the list of used components.

	The visible surfaces are classified into the following groups:

	1. surfaces that are not referenced in ComponentInstances
	2. ComponentInstance has not ComponentID (connection is made, component still needs assignment)
	3. ComponentInstance has invalid Component ID (could be automatically fixed to become 2.)
	4. ComponentInstance holds valid Componont IDs -> surfaces are grouped by Component

	The table contains entries for 1 to 3 (if any of such surfaces exist), and then for each of the entries in
	category 4.

	The following rules apply when assigning components to surfaces:

	- if the selected surface exists already in a ComponentInstance, this component instance is simply modified
	  (regardless of the potential other, not selected surface in the CI)
	- if the surface is not yet existant, a new CI is created

	Connecting two surfaces:

	- always two surfaces must be selected
	- if either of the surfaces is used in any ComponentInstance, these CI will be removed
	- a new CI is created with both surfaces and the selected component
*/
class SVPropBuildingComponentsWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingComponentsWidget(QWidget *parent = nullptr);
	~SVPropBuildingComponentsWidget();

	/*! Updates user interface. */
	void updateUi();

private slots:
	/*! Triggered when user switches component in table. */
	void on_tableWidgetComponents_itemSelectionChanged();

	/*! Triggers the openEditComponentDialog function */
	void on_pushButtonEditComponents_clicked();

	/*! Action to swap all occurances of currently selected component with newly selected component. */
	void on_pushButtonExchangeComponents_clicked();

	/*! All surfaces that reference the currently selected component (in the table) will be selected. */
	void on_pushButtonSelectObjectsWithComponent_clicked();

	/*! The user selects a component from the database and this will be assigned to all selected surfaces,
		possibly overwriting previously assigned components.
		If a surface does not yet have an association to a ComponentInstance, a new one will be created
		where the selected surface is assigned to side A.
	*/
	void on_pushButtonAssignComponent_clicked();

	/*! Assigns a component to two simultaneously selected surfaces. */
	void on_pushButtonAssignInsideComponent_clicked();

	/*! Assigns component from table to selected surfaces. */
	void on_pushButtonAssignComponentFromTable_clicked();

	/*! Triggers the openEditComponentDialog function */
	void on_tableWidgetComponents_cellDoubleClicked(int, int);

private:
	/*! This function opens the component DB dialog and lets the user select a component.
		Then, it creates new component instances for all selected surfaces.
		If insideWall is true, the two selected surfaces are connected to each other with an inside-wall-component.

		\param fromSurfaceSelection If false, the surfaces matching the currently selected component in the component
			table are modified, rather than the currently selected surfaces in the scene (used for Exchange Component).
		\param componentID If INVALID_ID, the user is prompted to select the component to be assigned, otherwise
			the function works silently and assigns the given component.
	*/
	void assignComponent(bool insideWall, bool fromSurfaceSelection, unsigned int componentID = VICUS::INVALID_ID);

	/*! Based on the current selection in the table widget, the corresponding component is looked up and returned.
		Only for valid component assignments.
	*/
	const VICUS::Component * currentlySelectedComponent() const;

	/*! Launches component db edit dialog. */
	void openEditComponentDialog();

	/*! Data structure that holds the information displayed in the table. */
	struct ComponentLegendEntry {
		// 0 = not connected, 1 = connected without component, 2 - connected with component
		int									m_type = 0;
		/*! Pointer to the component associated with the table entry (only for type 3). */
		const VICUS::Component				*m_component = nullptr;
		/*! Associated surfaces with this table entry.
			Note: each visible surfaces should only appear once in one of the legend entries
		*/
		std::vector<const VICUS::Surface*>	m_surfaces;
	};

	/*! Stores list of all selected surfaces. */
	std::vector<const VICUS::Surface * >	m_selectedSurfaces;

	/*! Stores the data shown in the table. */
	std::vector<ComponentLegendEntry>		m_componentTable;

	Ui::SVPropBuildingComponentsWidget *m_ui;
};

#endif // SVPropBuildingComponentsWidgetH
