#ifndef SVPropBuildingEditWidgetH
#define SVPropBuildingEditWidgetH

#include <QWidget>

namespace Ui {
	class SVPropBuildingEditWidget;
}

namespace VICUS {
	class Component;
	class Surface;
}

class ModificationInfo;

/*! A widget to edit site (and grid) related properties. */
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

private:
	Ui::SVPropBuildingEditWidget *m_ui;

	/*! This maps holds component->surface associations when in BM_Components mode. */
	std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> > m_componentSurfacesMap;

};


#endif // SVPropBuildingEditWidgetH
