#ifndef SVPropAcousticWidgetH
#define SVPropAcousticWidgetH

#include <QWidget>
#include "SVConstants.h"

namespace Ui {
class SVPropAcousticEditWidget;
}
class ModificationInfo;

class SVPropAcousticEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SVPropAcousticEditWidget(QWidget *parent = nullptr);
    ~SVPropAcousticEditWidget();
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

	/*! Connects to the ToolBox and changes the color view according to the currently selectd page */
	void onCurrentBuildingPropertyTypeChanged(int propertyType);

	/*! Returns current property type (index of toolbox) */
	unsigned int currentPropertyType();

	void onStyleChanged();

private:
	/*! Updates all edit widgets to current project state.
		This function gathers data needed by multiple edit widgets and then
		calls the updateUi() functions in the individual widgets.
	*/
	void updateUi();
	Ui::SVPropAcousticEditWidget *m_ui;
};

#endif // SVPropAcousticWidgetH
