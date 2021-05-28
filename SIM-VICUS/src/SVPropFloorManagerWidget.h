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

#ifndef SVPropFloorManagerWidgetH
#define SVPropFloorManagerWidgetH

#include <QWidget>

namespace Ui {
class SVPropFloorManagerWidget;
}

namespace VICUS {
	class Building;
	class BuildingLevel;
}

class ModificationInfo;
class QTreeWidgetItem;

/*! A widget to edit buildings/building levels and associate rooms with building levels. */
class SVPropFloorManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropFloorManagerWidget(QWidget *parent = nullptr);
	~SVPropFloorManagerWidget();


public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

private slots:
	void on_treeWidget_itemSelectionChanged();
	void on_pushButtonAddBuilding_clicked();
	void on_pushButtonAddLevel_clicked();
	void on_pushButtonRemoveBuilding_clicked();
	void on_pushButtonRemoveLevel_clicked();

	void on_pushButtonAssignRooms_clicked();

	void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

	void on_pushButtonAssignLevel_clicked();

private:
	Ui::SVPropFloorManagerWidget	*m_ui;

	/*! Pointer to currently selected building, updated in on_treeWidget_itemSelectionChanged(). */
	const VICUS::Building		*m_currentBuilding;
	/*! Pointer to currently selected building level, updated in on_treeWidget_itemSelectionChanged(). */
	const VICUS::BuildingLevel	*m_currentBuildingLevel;


};

#endif // SVPropFloorManagerWidgetH
