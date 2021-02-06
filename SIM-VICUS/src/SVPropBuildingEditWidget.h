#ifndef SVPropBuildingEditWidgetH
#define SVPropBuildingEditWidgetH

#include <QWidget>

namespace Ui {
	class SVPropBuildingEditWidget;
}

namespace VICUS {
	class Component;
	class Surface;
	class BoundaryCondition;
}

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

private:
	/*! Returns a pointer to the currently selected component in the component table. */
	const VICUS::Component * currentlySelectedComponent() const;

	/*! Called when the visibility/selection state of objects has changed.
		Updates the selection-specific properties.
	*/
	void objectSelectionChanged();

	Ui::SVPropBuildingEditWidget	*m_ui;

	/*! Stores the current property type (set in setPropertyType()). */
	int								m_propertyType = -1;
	/*! This maps holds component->surface associations when in BM_Components mode. */
	std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >			m_componentSurfacesMap;
	/*! This map links boundary conditions to surfaces. */
	std::map<const VICUS::BoundaryCondition*, std::vector<const VICUS::Surface *> > m_bcSurfacesMap;

};


#endif // SVPropBuildingEditWidgetH
