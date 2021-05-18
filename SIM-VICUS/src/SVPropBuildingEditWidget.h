#ifndef SVPropBuildingEditWidgetH
#define SVPropBuildingEditWidgetH

#include <QWidget>

#include <set>

namespace Ui {
	class SVPropBuildingEditWidget;
}

namespace VICUS {
	class Component;
	class ComponentInstance;
	class SubSurface;
	class Surface;
	class BoundaryCondition;
	class ZoneTemplate;
	class Room;
	class SubSurfaceComponentInstance;
	class SubSurfaceComponent;
}

class QTableWidgetItem;
class ModificationInfo;

/*! A widget to edit building properties.
	The type of property to be edited is set with function setPropertyType(). This switches between
	different widgets (in the stacked widget).

	Note: the highlighting mode of the scene is not set here, but rather in the edit mode
	selection widget.

	The content of the tables is updated whenever the respective data changes. This is somewhat tricky,
	since some properties (like component colors) are not part of the project, but user-database content.

	Hence, we need to listen to the following change events:

	Components:
	- changes in project (might be a component association)
	- changes in selection (updates selected component info)
	- changes in component DB
*/
class SVPropBuildingEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingEditWidget(QWidget *parent = nullptr);
	~SVPropBuildingEditWidget();

	/*! Switches property widget into specific mode.
		\param buildingPropertyType Type of selected (building) property, see BuildingPropertyTypes
	*/
	void setPropertyType(int buildingPropertyType);

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

	/*! Connected to SVViewHandler::colorRefreshNeeded() and is triggered whenever a database element was modified
		which means that the color in the tables may have changed.
		Basically updates the current user interface as if the selection-combo at the top of the property widgets has
		changed.
	*/
	void onColorRefreshNeeded();

private slots:

	// *** Component property mode ***

	/*! Launches component db edit dialog. */
	void on_pushButtonEditComponents_clicked();

	/*! Action to swap all occurances of currently selected component with newly selected component. */
	void on_pushButtonExchangeComponents_clicked();

	/*! Triggered when user switches component in table. */
	void on_tableWidgetComponents_itemSelectionChanged();

	/*! All surfaces that reference the currently selected component (in the table) will be selected. */
	void on_pushButtonSelectObjectsWithComponent_clicked();

	/*! The user selects a component from the database and this will be assigned to all selected surfaces,
		possibly overwriting previously assigned components.
		If a surface does not yet have an association to a ComponentInstance, a new one will be created
		where the selected surface is assigned to side A.
	*/
	void on_pushButtonAssignComponent_clicked();

	void on_checkBoxShowAllComponentOrientations_toggled(bool checked);

	void on_pushButtonAlignComponentToSideA_clicked();

	void on_pushButtonAlignComponentToSideB_clicked();

	void on_comboBoxComponentSelection_currentIndexChanged(int);

	void on_pushButtonAssignInsideComponent_clicked();



	void on_tableWidgetSubSurfaceComponents_itemSelectionChanged();



	void on_pushButtonAssignZoneTemplate_clicked();

	void on_tableWidgetZoneTemplates_itemSelectionChanged();

	void on_pushButtonEditZoneTemplates_clicked();

	void on_pushButtonExchangeZoneTemplates_clicked();

	void on_checkBoxZoneTemplateColorOnlyActive_toggled(bool checked);

	void on_checkBoxZoneTemplateShowOnlyActive_toggled(bool checked);

	void on_tableWidgetZoneTemplates_itemClicked(QTableWidgetItem *item);

	void on_pushButtonEditSubSurfaceComponents_clicked();

private:
	/*! Returns a pointer to the currently selected component in the component table. */
	const VICUS::Component * currentlySelectedComponent() const;
	/*! Returns a pointer to the currently selected sub-surface component in the sub-surface component table. */
	const VICUS::SubSurfaceComponent * currentlySelectedSubSurfaceComponent() const;
	/*! Returns a pointer to the currently selected zone template in the zone template table. */
	const VICUS::ZoneTemplate * currentlySelectedZoneTemplate() const;

	/*! Updates widget to current project state. */
	void updateUi();

	/*! This function opens the component DB dialog and lets the user select a component.
		Then, it creates new component instances for all selected surfaces.
		If insideWall is true, the two selected surfaces are connected to each other with an inside-wall-component.
	*/
	void assignComponent(bool insideWall);

	/*! This function toggles side assignments in component instances for selected components. */
	void alignSelectedComponents(bool toSideA);

	/*! Triggered when show-only-active check box was checked and table selection has changed,
		sends a recolor signal.
	*/
	void zoneTemplateVisibilityChanged();

	/*! Triggered when select-active check box was checked and table selection has changed,
		modifies visibility state of respective room nodes (and their surfaces).
	*/
	void zoneTemplateSelectionChanged();



	Ui::SVPropBuildingEditWidget	*m_ui;

	/*! Stores the current property type (set in setPropertyType()). */
	int								m_propertyType = -1;

	/*! This maps holds component->surface associations when in BM_Components mode. */
	std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >			m_componentSurfacesMap;
	/*! This map links boundary conditions to surfaces. */
	std::map<const VICUS::BoundaryCondition*, std::vector<const VICUS::Surface *> > m_bcSurfacesMap;
	/*! This set contains all shown component instances that have one or more sides selected. */
	std::set<const VICUS::ComponentInstance*>										m_selectedComponentInstances;

	/*! This maps holds component->surface associations when in BM_SubSurfaceComponents mode. */
	std::map<const VICUS::SubSurfaceComponent*, std::vector<const VICUS::SubSurface *> >	m_subComponentSurfacesMap;
	/*! This map links boundary conditions to surfaces. */
	std::map<const VICUS::BoundaryCondition*, std::vector<const VICUS::SubSurface *> >	m_bcSubSurfacesMap;
	/*! This set contains all shown component instances that have one or more sides selected. */
	std::set<const VICUS::SubSurfaceComponentInstance*>									m_selectedSubComponentInstances;


	/*! Maps stores pointers to room objects grouped for assigned zone templates.
		Note: rooms without zone template ID are ignored.
	*/
	std::map<const VICUS::ZoneTemplate*, std::vector<const VICUS::Room *> >			m_zoneTemplateAssignments;

};


#endif // SVPropBuildingEditWidgetH
