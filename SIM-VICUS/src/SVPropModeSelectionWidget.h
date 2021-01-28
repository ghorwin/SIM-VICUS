#ifndef SVPropModeSelectionWidgetH
#define SVPropModeSelectionWidgetH

#include <QWidget>

#include "SVConstants.h"

namespace Ui {
	class SVPropModeSelectionWidget;
}

class ModificationInfo;
class SVViewState;


/*! This widget is shown on top of the property widget page when in property-edit mode.
	In this widget the user selects the basic editing modes.

	In addition to manual change by user, the property selection combo boxes can change when
	user has switched the edit mode (to building, to network), or when the object selection changes.
	This is done in function selectionChanged().

	Whenever the edit mode has changed (basic mode or property combo box value), a new
	viewstate is set in the view state handler.
*/
class SVPropModeSelectionWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropModeSelectionWidget(QWidget *parent = nullptr);
	~SVPropModeSelectionWidget();

	/*! Returns the currently selected choice in the building property selection combo. */
	BuildingPropertyTypes currentBuildingPropertyType() const;

	/*! Returns the currently selected choice in the network property selection combo. */
	int currentNetworkPropertyType() const;

	/*! Sets the properties m_propertyWidgetMode and m_objectColorMode based on current selections in the widget. */
	void viewStateProperties(SVViewState & vs) const;

public slots:
	/*! Connected to SVProjectHandler::modified() */
	void onModified(int modificationType, ModificationInfo * );

private slots:
	void on_pushButtonBuilding_toggled(bool checked);

	void on_pushButtonNetwork_toggled(bool checked);

	void on_pushButtonSite_toggled(bool checked);

	void on_comboBoxNetworkProperties_currentIndexChanged(int);

	void on_comboBoxBuildingProperties_currentIndexChanged(int);

private:
	/*! Based on selected properties, switch to one of the specific edit modes, based on
		the selected objects. Sends only out a view state change event, if the combo box value
		has been modified.
	*/
	void selectionChanged();

	/*! This is called whenever the user has made changes to any of the components in
		this widget and updates the enabled/disabled states of all controls.
	*/
	void updateWidgetVisibility();

	/*! Sets a new view state in the view state handler, which causes the rest of the
		user interface to update its states accordingly.
		This function is called from the slots and selectionChanged().
	*/
	void updateViewState();

	Ui::SVPropModeSelectionWidget *m_ui;
};


#endif // SVPropModeSelectionWidgetH
