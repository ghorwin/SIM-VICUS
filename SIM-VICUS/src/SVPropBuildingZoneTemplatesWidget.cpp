#include "SVPropBuildingZoneTemplatesWidget.h"
#include "ui_SVPropBuildingZoneTemplatesWidget.h"

#include <SVConversions.h>

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"

#include "SVUndoModifyRoomZoneTemplateAssociation.h"
#include "SVViewStateHandler.h"
#include "SVUndoTreeNodeState.h"
#include "SVMainWindow.h"
#include "SVDBZoneTemplateEditDialog.h"

SVPropBuildingZoneTemplatesWidget::SVPropBuildingZoneTemplatesWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingZoneTemplatesWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetZoneTemplates->setColumnCount(2);
	m_ui->tableWidgetZoneTemplates->setHorizontalHeaderLabels(QStringList() << QString() << tr("Zone template"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetZoneTemplates);
	m_ui->tableWidgetZoneTemplates->setSortingEnabled(false);
	m_ui->tableWidgetZoneTemplates->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetZoneTemplates->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetZoneTemplates->horizontalHeader()->setStretchLastSection(true);
}


SVPropBuildingZoneTemplatesWidget::~SVPropBuildingZoneTemplatesWidget() {
	delete m_ui;
}


void SVPropBuildingZoneTemplatesWidget::updateUi() {
	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // visible (selected and not selected)

	m_zoneTemplateAssignments.clear();
	const VICUS::Database<VICUS::ZoneTemplate> & db_zt = SVSettings::instance().m_db.m_zoneTemplates;
	// loop over all rooms and store zone template associations
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		// skip rooms without zone template
		if (room->m_idZoneTemplate == VICUS::INVALID_ID)
			continue;
		// lookup zone template in DB
		const VICUS::ZoneTemplate * zt = db_zt[room->m_idZoneTemplate];
		// Note: might be a nullptr if id is invalid
		m_zoneTemplateAssignments[zt].push_back(room);
	}


	// now put the data of the map into the table
	int currentRow = m_ui->tableWidgetZoneTemplates->currentRow();
	m_ui->tableWidgetZoneTemplates->blockSignals(true);
	m_ui->tableWidgetZoneTemplates->clearContents();
	m_ui->tableWidgetZoneTemplates->setRowCount(m_zoneTemplateAssignments.size());
	int row=0;
	for (std::map<const VICUS::ZoneTemplate*, std::vector<const VICUS::Room *> >::const_iterator
		 it = m_zoneTemplateAssignments.begin(); it != m_zoneTemplateAssignments.end(); ++it, ++row)
	{
		QTableWidgetItem * item = new QTableWidgetItem();
		// special handling for zone template with "invalid" id
		if (it->first == nullptr)
			item->setBackground(QColor(64,64,64));
		else
			item->setBackground(it->first->m_color);
		item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		m_ui->tableWidgetZoneTemplates->setItem(row, 0, item);

		item = new QTableWidgetItem();
		if (it->first == nullptr)
			item->setText(tr("<invalid zone template id>"));
		else
			item->setText(QtExt::MultiLangString2QString(it->first->m_displayName) );
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetZoneTemplates->setItem(row, 1, item);
	}
	m_ui->tableWidgetZoneTemplates->blockSignals(false);
	m_ui->tableWidgetZoneTemplates->selectRow(std::min(currentRow, m_ui->tableWidgetZoneTemplates->rowCount()-1));

	// process all selected rooms and determine which zone template they have assigned
	std::vector<const VICUS::Room*> rooms;
	project().selectedRooms(rooms);
	if (rooms.empty()) {
		m_ui->labelSelectedZoneTemplates->setText("");
		m_ui->groupBoxSelectedRooms->setEnabled(false);
	}
	else {
		m_ui->groupBoxSelectedRooms->setEnabled(true);
	}

	// update push button for selected zone assignement
	m_ui->pushButtonAssignSelectedZoneTemplate->setEnabled(!rooms.empty());
	m_ui->pushButtonAssignSelectedZoneTemplate->setEnabled(m_ui->tableWidgetZoneTemplates->rowCount() == 1);

	const SVDatabase & db = SVSettings::instance().m_db;

	// update selection-related info
	std::set<const VICUS::ZoneTemplate *> selectedZoneTemplate;
	// loop over all selected rooms and store pointer to assigned zone template
	for (const VICUS::Room* r : rooms) {
		if (r->m_idZoneTemplate != VICUS::INVALID_ID) {
			const VICUS::ZoneTemplate * zt= db.m_zoneTemplates[r->m_idZoneTemplate];
			selectedZoneTemplate.insert(zt); // when ID is invalid/unknown, we store a nullptr
		}
	}
	if (selectedZoneTemplate.empty()) {
		m_ui->labelSelectedZoneTemplates->setText(tr("None"));
	}
	else if (selectedZoneTemplate.size() == 1) {
		const VICUS::ZoneTemplate * zt = *selectedZoneTemplate.begin();
		// special handling: exactly one room with invalid zone template ID is selected
		if (zt == nullptr)
			m_ui->labelSelectedZoneTemplates->setText(tr("Zone template with invalid/unknown ID"));
		else // otherwise show info about the selected zone template
			m_ui->labelSelectedZoneTemplates->setText(tr("%1 [%2]")
			   .arg(QtExt::MultiLangString2QString(zt->m_displayName)).arg(zt->m_id) );
	}
	else {
		m_ui->labelSelectedZoneTemplates->setText(tr("%1 different templates")
		   .arg(selectedZoneTemplate.size()));
	}
	// update table related button states
	on_tableWidgetZoneTemplates_itemSelectionChanged();
}


void SVPropBuildingZoneTemplatesWidget::zoneTemplateSelectionChanged() {
	// do nothing, if checkbox isn't active; that also means that when the checkbox is unchecked,
	// we will not automatically show all hidden objects again
	if (!m_ui->checkBoxZoneTemplateShowOnlyActive->isChecked())
		return;

	// compose node states for all rooms and their surfaces based on template association

	// get currently selected zone template
	const VICUS::ZoneTemplate * zt = currentlySelectedZoneTemplate();

	// if not a valid template, do nothing here
	if (zt == nullptr)
		return;

	// compose a list of unique room IDs and respective on/off visibility states
	std::set<unsigned int> nodeIDs;

	const VICUS::Project & p = project();

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				// skip all rooms that do not have the same zone template as we have
				if (r.m_idZoneTemplate != zt->m_id) continue;
				nodeIDs.insert(r.m_id); // Mind: unique IDs!
				for (const VICUS::Surface & s : r.m_surfaces)
					nodeIDs.insert(s.m_id); // Mind: unique IDs!
			}
		}
	}

	// trigger undo-action
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Exclusive selection by zone template"), SVUndoTreeNodeState::VisibilityState, nodeIDs, true, true);
	undo->push();
}




void SVPropBuildingZoneTemplatesWidget::on_pushButtonAssignZoneTemplate_clicked() {
	// ask user to select a the zone template to assign
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignZoneTemplate",
		tr("Assign zone template"), tr("You may now select a zone template from the database, which will then be "
								   "assigned to the selected rooms."));
	unsigned int selectedId = SVMainWindow::instance().dbZoneTemplateEditDialog()->select(0);
	if (selectedId == VICUS::INVALID_ID)
		return; // user aborted the dialog

	// get all visible _and_ selected "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, true, true);

	std::vector<unsigned int> modifiedRoomIDs; // unique IDs!!!
	// loop over all rooms and store zone template associations
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		modifiedRoomIDs.push_back(room->m_id);
	}
	// now create an undo action for modifying zone template assignments
	SVUndoModifyRoomZoneTemplateAssociation * undo = new SVUndoModifyRoomZoneTemplateAssociation(
				tr("Assigned zone template"),
				modifiedRoomIDs, selectedId);
	undo->push();
}


void SVPropBuildingZoneTemplatesWidget::on_tableWidgetZoneTemplates_itemSelectionChanged() {
	// enable/disable buttons based on selection changed signal
	bool enabled = (currentlySelectedZoneTemplate() != nullptr);
	m_ui->pushButtonEditZoneTemplates->setEnabled(enabled);
	m_ui->pushButtonExchangeZoneTemplates->setEnabled(enabled);
	m_ui->pushButtonSelectObjectsWithZoneTemplate->setEnabled(enabled);

	// the assign-from-table button is only available when there is at least one surface selected
	m_ui->pushButtonAssignSelectedZoneTemplate->setEnabled(enabled);
}


void SVPropBuildingZoneTemplatesWidget::on_pushButtonEditZoneTemplates_clicked() {
	const VICUS::ZoneTemplate * zt = currentlySelectedZoneTemplate();
	Q_ASSERT(zt != nullptr); // if nullptr, the button should be disabled!
	SVMainWindow::instance().dbZoneTemplateEditDialog()->edit(zt->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
}


void SVPropBuildingZoneTemplatesWidget::on_pushButtonExchangeZoneTemplates_clicked() {
	const VICUS::ZoneTemplate * zt = currentlySelectedZoneTemplate();
	Q_ASSERT(zt != nullptr); // if nullptr, the button should be disabled!
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceComponent",
		tr("Replace template"), tr("This will replace all associations with zone template '%1 [%2]' with another template.")
			 .arg(QtExt::MultiLangString2QString(zt->m_displayName)).arg(zt->m_id));
	unsigned int oldId = zt->m_id;
	unsigned int newId = SVMainWindow::instance().dbZoneTemplateEditDialog()->select(oldId);
	if (newId == VICUS::INVALID_ID)
		return; // user has aborted the dialog

	// now compose an undo action and modify the project

	// first, we need to find the component instances which reference the "old" id
	zt = SVSettings::instance().m_db.m_zoneTemplates[oldId];
	if (zt == nullptr) {
		// the user has done something stupid and deleted the component that he wanted to replace. In this
		// case there is no longer a component with this ID and we have a nullptr
		QMessageBox::critical(this, tr("Replace template"), tr("Zone template with id %1 no longer found in DB.").arg(oldId));
		return;
	}

	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true);

	std::vector<unsigned int> modifiedRoomIDs; // unique IDs!!!
	// loop over all rooms and if their current zone template matches the oldId, remember the room to be modified
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		if (room->m_idZoneTemplate == oldId)
			modifiedRoomIDs.push_back(room->m_id); // Mind: unique IDs!
	}


	// create the undo action and modify project
	// now create an undo action for modifying zone template assignments
	SVUndoModifyRoomZoneTemplateAssociation * undo = new SVUndoModifyRoomZoneTemplateAssociation(
				tr("Exchange zone template"),
				modifiedRoomIDs, newId);
	undo->push();
}



void SVPropBuildingZoneTemplatesWidget::on_checkBoxZoneTemplateColorOnlyActive_toggled(bool) {
	zoneTemplateVisibilityChanged();
}


void SVPropBuildingZoneTemplatesWidget::on_checkBoxZoneTemplateShowOnlyActive_toggled(bool) {
	zoneTemplateSelectionChanged();
}


void SVPropBuildingZoneTemplatesWidget::on_tableWidgetZoneTemplates_itemClicked(QTableWidgetItem *) {
	if (m_ui->checkBoxZoneTemplateColorOnlyActive->isChecked())
		zoneTemplateVisibilityChanged();
	if (m_ui->checkBoxZoneTemplateShowOnlyActive->isChecked())
		zoneTemplateSelectionChanged();
}

void SVPropBuildingZoneTemplatesWidget::on_pushButtonSelectObjectsWithZoneTemplate_clicked() {
	int row = m_ui->tableWidgetZoneTemplates->currentRow();
	Q_ASSERT(row != -1);

	std::map<const VICUS::ZoneTemplate*, std::vector<const VICUS::Room *> >::const_iterator cit = m_zoneTemplateAssignments.begin();
	std::advance(cit, row);
	const VICUS::ZoneTemplate * zt = cit->first;
	Q_ASSERT(m_zoneTemplateAssignments.find(zt) != m_zoneTemplateAssignments.end());

	std::set<unsigned int> objIds;
	for(const VICUS::Room *r : m_zoneTemplateAssignments[zt]){
		objIds.insert(r->m_id);
		for (const VICUS::Surface &s : r->m_surfaces)
			objIds.insert(s.m_id);
	}

	QString undoText;
	if (zt != nullptr)
		undoText = tr("Select objects with zone template '%1'").arg(QtExt::MultiLangString2QString(zt->m_displayName));
	else
		undoText = tr("Select objects with invalid/missing zone template.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, objIds, true);
	undo->push();
}


const VICUS::ZoneTemplate * SVPropBuildingZoneTemplatesWidget::currentlySelectedZoneTemplate() const {
	// check if selected "template" is actually missing
	int r = m_ui->tableWidgetZoneTemplates->currentRow();
	if (r == -1 || m_zoneTemplateAssignments.empty())
		return nullptr;
	std::map<const VICUS::ZoneTemplate*, std::vector<const VICUS::Room *> >::const_iterator cit = m_zoneTemplateAssignments.begin();
	std::advance(cit, r);
	return cit->first;
}

void SVPropBuildingZoneTemplatesWidget::zoneTemplateVisibilityChanged() {
	// set currently selected zone template ID in view state and trigger a recoloring
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (m_ui->checkBoxZoneTemplateColorOnlyActive->isChecked()) {
		const VICUS::ZoneTemplate * selectedZoneTemplate = currentlySelectedZoneTemplate();
		if (selectedZoneTemplate == nullptr)
			vs.m_colorModePropertyID = VICUS::INVALID_ID;
		else
			vs.m_colorModePropertyID = selectedZoneTemplate->m_id;
	}
	else {
		// disable coloring filter
		vs.m_colorModePropertyID = VICUS::INVALID_ID;
	}
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropBuildingZoneTemplatesWidget::on_pushButtonAssignSelectedZoneTemplate_clicked() {

	// find out which component is selected in table
	// get currently selected zone template
	const VICUS::ZoneTemplate * zt = currentlySelectedZoneTemplate();

	// if not a valid template, do nothing here
	if (zt == nullptr)
		return;

	// get all visible _and_ selected "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, true, true);

	std::vector<unsigned int> modifiedRoomIDs; // unique IDs!!!
	// loop over all rooms and store zone template associations
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		modifiedRoomIDs.push_back(room->m_id);
	}
	// now create an undo action for modifying zone template assignments
	SVUndoModifyRoomZoneTemplateAssociation * undo = new SVUndoModifyRoomZoneTemplateAssociation(
				tr("Assigned zone template"),
				modifiedRoomIDs, zt->m_id);
	undo->push();
}

