#include "SVPropBuildingStructuralUnitWidget.h"
#include "ui_SVPropBuildingStructuralUnitWidget.h"

#include <SVConversions.h>
#include <QInputDialog>

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVUndoModifyStructuralUnitRoomAssociation.h"
#include "SVUndoDeleteStructuralUnit.h"
#include "SVStructuralUnitCreationDialog.h"
#include "SVMainWindow.h"



SVPropBuildingStructuralUnitWidget::SVPropBuildingStructuralUnitWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingStructuralUnitWidget)
{
	m_ui->setupUi(this);

	// set layout of the table widget
	m_ui->tableWidgetStructuralUnit->setColumnCount(2);
	m_ui->tableWidgetStructuralUnit->setHorizontalHeaderLabels(QStringList() << QString() << tr("Structural Unit"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetStructuralUnit);
	m_ui->tableWidgetStructuralUnit->setSortingEnabled(false);
	m_ui->tableWidgetStructuralUnit->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetStructuralUnit->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetStructuralUnit->horizontalHeader()->setStretchLastSection(true);

	// update Ui initiallly
	updateUi();
}

SVPropBuildingStructuralUnitWidget::~SVPropBuildingStructuralUnitWidget()
{
	delete m_ui;
}




void SVPropBuildingStructuralUnitWidget::updateUi() {



	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, false); // we always show all in table

	// now put the data of the map into the table
	int currentRow = m_ui->tableWidgetStructuralUnit->currentRow();
	m_ui->tableWidgetStructuralUnit->blockSignals(true);
	m_ui->tableWidgetStructuralUnit->setRowCount(0);
	int row=0;

	for(const VICUS::StructuralUnit & unit : project().m_structuralUnits){
		m_ui->tableWidgetStructuralUnit->setRowCount(row + 1);

		QTableWidgetItem * item = new QTableWidgetItem();
		item->setBackground(unit.m_color);
		item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		m_ui->tableWidgetStructuralUnit->setItem(row, 0, item);

		item = new QTableWidgetItem();
		item->setText(unit.m_displayName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetStructuralUnit->setItem(row, 1, item);

		row++;
	}

	m_ui->tableWidgetStructuralUnit->blockSignals(false);
	m_ui->tableWidgetStructuralUnit->selectRow(std::min(currentRow, m_ui->tableWidgetStructuralUnit->rowCount()-1));

	// process all selected rooms and determine which zone template they have assigned
	std::vector<const VICUS::Room*> rooms;
	project().selectedRooms(rooms);
	if (rooms.empty()) {
		m_ui->labelSelectedStructuralUnit->setText("");
		m_ui->groupBoxSelectedRooms->setEnabled(false);
	}
	else {
		m_ui->groupBoxSelectedRooms->setEnabled(true);
	}

	// update push button for selected zone assignement
	m_ui->pushButtonAssignStructuralUnit->setEnabled(!rooms.empty());
	m_ui->pushButtonAssignStructuralUnit->setEnabled(m_ui->tableWidgetStructuralUnit->rowCount() == 1);

	// update selection-related info
	std::set<const VICUS::StructuralUnit *> selectedStructuralUnit;
	std::set<const VICUS::Room *> selectedRoomsWithStructuralUnit;

	for(const VICUS::StructuralUnit & unit : project().m_structuralUnits){
		for(unsigned int id : unit.m_roomIds){
			for(const VICUS::Room * r : rooms){
				if(r->m_id == id){
					selectedStructuralUnit.insert(&unit);
					selectedRoomsWithStructuralUnit.insert(r);
				}
			}
		}
	}



	if (selectedStructuralUnit.empty()) {
		m_ui->labelSelectedStructuralUnit->setText(tr("None"));
	}
	// display information about selected structural unit when only is selected and no rooms without one
	else if (selectedStructuralUnit.size() == 1 && selectedRoomsWithStructuralUnit.size() == rooms.size()) {
		const VICUS::StructuralUnit * unit = *selectedStructuralUnit.begin();
		// special handling: exactly one room with invalid zone template ID is selected
		if (unit == nullptr)
			m_ui->labelSelectedStructuralUnit->setText(tr("Structural unit with invalid/unknown ID"));
		else // otherwise show info about the selected structural unit
			m_ui->labelSelectedStructuralUnit->setText(tr("%1 [%2]")
													   .arg(unit->m_displayName).arg(unit->m_id) );
	}
	else {
		if(selectedRoomsWithStructuralUnit.size() != rooms.size()){
			m_ui->labelSelectedStructuralUnit->setText(tr("Room without structural unit selected"));
		} else {
			m_ui->labelSelectedStructuralUnit->setText(tr("%1 different templates")
													   .arg(selectedStructuralUnit.size()));
		}
	}



	// update table related button states
	on_tableWidgetStructuralUnit_itemSelectionChanged();
}


const VICUS::StructuralUnit * SVPropBuildingStructuralUnitWidget::currentlySelectedStructuralUnit() const {
	// check if selected "template" is actually missing
	int r = m_ui->tableWidgetStructuralUnit->currentRow();
	if (r == -1)
		return nullptr;
	return &project().m_structuralUnits[(unsigned int)r];
}




void SVPropBuildingStructuralUnitWidget::on_tableWidgetStructuralUnit_itemSelectionChanged()
{
	// the assign-from-table button is only available when there is at least one surface selected
	m_ui->pushButtonAssignStructuralUnit->setEnabled(currentlySelectedStructuralUnit() != nullptr);

	//enable remove button if a structural unit is selected
	m_ui->pushButtonRemoveStructuralUnit->setEnabled(currentlySelectedStructuralUnit() != nullptr);


}

void SVPropBuildingStructuralUnitWidget::on_pushButtonAddStructuralUnit_clicked() {
    SVMainWindow::instance().structuralUnitDialog()->create();
	// make changes visible
	updateUi();
}


void SVPropBuildingStructuralUnitWidget::on_pushButtonAssignStructuralUnit_clicked() {
	// find out which component is selected in table
	// get currently selected acoustic template
	const VICUS::StructuralUnit * unit = currentlySelectedStructuralUnit();

	// if not a valid unit, do nothing here
	if (unit == nullptr)
		return;

	// get all visible _and_ selected "building" type objects in the scene
	std::vector<const VICUS::Room*> rooms;
	project().selectedRooms(rooms);

	std::set<unsigned int> roomIDs;
	// loop over all rooms and store acoustic template associations
	for (const VICUS::Room * ro : rooms)
		roomIDs.insert(ro->m_id);

	// now create an undo action for modifying zone template assignments
	SVUndoAddStructuralUnitRoomAssociation * undo = new SVUndoAddStructuralUnitRoomAssociation(
				tr("Assigned Structural Unit"),
				roomIDs, const_cast<VICUS::StructuralUnit * >(currentlySelectedStructuralUnit()));
	undo->push();
}


void SVPropBuildingStructuralUnitWidget::on_tableWidgetStructuralUnit_cellDoubleClicked() {
    SVMainWindow::instance().structuralUnitDialog()->edit(currentlySelectedStructuralUnit());
    // input ändern zu direkten structural unit
	// make changes visible
	updateUi();
}


void SVPropBuildingStructuralUnitWidget::on_pushButtonRemoveStructuralUnit_clicked() {
	const VICUS::StructuralUnit * selectedUnit = currentlySelectedStructuralUnit();
	SVUndoDeleteStructuralUnit * undo = new SVUndoDeleteStructuralUnit(
				tr("Deleting Structural Unit [%1]").arg(selectedUnit->m_id),
				*selectedUnit);
	undo->push();
}


void SVPropBuildingStructuralUnitWidget::on_pushButtonRemoveAssignment_clicked() {
	// get all ids of the selected rooms
	std::set<unsigned int> roomIDs;
	std::vector<const VICUS::Room*> rooms;
	project().selectedRooms(rooms);
	// loop over all rooms and store acoustic template associations
	for (const VICUS::Room * ro : rooms)
		roomIDs.insert(ro->m_id);

	// insert the ids that need to be removed (the structural unit does not matter)
	std::set<unsigned int> removedIds;
	//check which units need to modified when these rooms get deselected
	for(const VICUS::StructuralUnit & unit : project().m_structuralUnits){
		for(unsigned int id : unit.m_roomIds){
			for(unsigned int selectedId : roomIDs){
				if(id == selectedId){
					removedIds.insert(id);
				}
			}
		}
	}
	SVUndoRemoveStructuralUnitRoomAssociation * undo = new SVUndoRemoveStructuralUnitRoomAssociation(
				tr("Removing rooms associations"), removedIds);
	undo->push();
}

