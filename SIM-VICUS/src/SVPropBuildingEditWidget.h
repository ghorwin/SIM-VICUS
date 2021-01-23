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
	void on_toolButtonEdit_clicked();

	void on_pushButtonEditComponents_clicked();

	void on_pushButtonExchangeComponents_clicked();

	void on_tableWidgetComponents_itemSelectionChanged();

private:
	Ui::SVPropBuildingEditWidget *m_ui;

	/*! This maps holds component->surface associations when in BM_Components mode. */
	std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> > m_componentSurfacesMap;
	std::map<const VICUS::BoundaryCondition*, std::vector<const VICUS::Surface *> > m_bcSurfacesMap;

};


#endif // SVPropBuildingEditWidgetH
