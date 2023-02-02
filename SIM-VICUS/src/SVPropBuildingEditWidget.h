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

#ifndef SVPropBuildingEditWidgetH
#define SVPropBuildingEditWidgetH

#include <QWidget>

#include <set>

#include "SVConstants.h"


namespace Ui {
	class SVPropBuildingEditWidget;
}

class ModificationInfo;


/*!
 *  This class owns the ToolBox and pointers to all building editing widgets, which are used in the ToolBox. It also provides them with the currently selected objects.
 *  It reacts on project changes (and selection changes), updates the editing widgets and also updates th color view.
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

	/*! Connects to the ToolBox and changes the color view according to the currently selectd page */
	void onCurrentBuildingPropertyTypeChanged(int propertyType);

	/*! Returns current property type (index of toolbox) */
	unsigned int currentPropertyType();


private:
	/*! Updates all edit widgets to current project state.
		This function gathers data needed by multiple edit widgets and then
		calls the updateUi() functions in the individual widgets.
	*/
	void updateUi(bool onlyNodeStateModified);

	Ui::SVPropBuildingEditWidget	*m_ui;
};


#endif // SVPropBuildingEditWidgetH
