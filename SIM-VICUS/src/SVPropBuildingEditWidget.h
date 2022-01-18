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

namespace Ui {
	class SVPropBuildingEditWidget;
}

class ModificationInfo;

/*! A widget to edit building properties.
	This widget is actually just a container for all the individual property widgets.
	The type of property to be edited is set with function setPropertyType(), called from
	SVPropertyWidget::onViewStateChanged(). This switches between different widgets (in the stacked widget).

	Note: the highlighting mode of the scene is not set here.
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

private:
	/*! Updates all edit widgets to current project state.
		This function gathers data needed by multiple edit widgets and then
		calls the updateUi() functions in the individual widgets.
	*/
	void updateUi();

	Ui::SVPropBuildingEditWidget	*m_ui;
	/*! Stores the current property type (set in setPropertyType()). */
	int								m_propertyType = -1;
};


#endif // SVPropBuildingEditWidgetH
