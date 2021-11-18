/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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

	/*! Checks the "Building" button and selects the requested building property type programmatically, but without
		firing any signals.
	*/
	void setBuildingPropertyType(BuildingPropertyTypes pt);

	/*! Returns the currently selected choice in the building property selection combo. */
	BuildingPropertyTypes currentBuildingPropertyType() const;

	/*! Returns the currently selected choice in the network property selection combo. */
	int currentNetworkPropertyType() const;

	/*! Sets the properties m_propertyWidgetMode and m_objectColorMode based on current selections in the widget. */
	void viewStateProperties(SVViewState & vs) const;

	/*! returns currently selected network id */
	unsigned int currentNetworkId() const;

	/*! Switches the network selection combo box to the entry with the given networkId. */
	void setCurrentNetwork(unsigned networkId);

	/*! Sets a meaningful view state based on current's property widget appearance, and whether we have selection or not.
		This function is called when we turn off any intermediate modes, like "align coordinate system"-mode.
	*/
	void setDefaultViewState();


public slots:
	/*! Connected to SVProjectHandler::modified() */
	void onModified(int modificationType, ModificationInfo * );

private slots:
	void on_pushButtonBuilding_toggled(bool checked);

	void on_pushButtonNetwork_toggled(bool checked);

	void on_pushButtonSite_toggled(bool checked);

	void on_comboBoxNetworkProperties_currentIndexChanged(int);

	void on_comboBoxBuildingProperties_currentIndexChanged(int);

	void on_comboBoxSelectedNetwork_currentIndexChanged(int index);

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
