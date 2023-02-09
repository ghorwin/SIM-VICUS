#include "SVPropBuildingAcousticTemplatesWidget.h"
#include "ui_SVPropBuildingAcousticTemplatesWidget.h"

#include <SVConversions.h>

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyRoomAcousticTemplateAssociation.h"


SVPropBuildingAcousticTemplatesWidget::SVPropBuildingAcousticTemplatesWidget(QWidget *parent) :
    QWidget(parent),
	m_ui(new Ui::SVPropBuildingAcousticTemplatesWidget)
{
	m_ui->setupUi(this);

	// set layout of the table widget
	m_ui->tableWidgetAcousticTemplates->setColumnCount(3);
	m_ui->tableWidgetAcousticTemplates->setHorizontalHeaderLabels(QStringList() << QString() << tr("Acoustic template") << tr("Note"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetAcousticTemplates);
	m_ui->tableWidgetAcousticTemplates->setSortingEnabled(false);
	m_ui->tableWidgetAcousticTemplates->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetAcousticTemplates->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetAcousticTemplates->horizontalHeader()->resizeSection(1,220);
	m_ui->tableWidgetAcousticTemplates->horizontalHeader()->setStretchLastSection(true);

}

SVPropBuildingAcousticTemplatesWidget::~SVPropBuildingAcousticTemplatesWidget()
{
	delete m_ui;
}

void SVPropBuildingAcousticTemplatesWidget::updateUi() {
	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, false); // we always show all in table


	const VICUS::Database<VICUS::AcousticTemplate> & db_at = SVSettings::instance().m_db.m_acousticTemplates;

	// is not used at the moment
//	m_acousticTemplateAssignments.clear();
//	for (const VICUS::Object * o : objs) {
//		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
//		if (room == nullptr) continue; // skip all but rooms
//		// skip rooms without zone template
//		if (room->m_idAcousticTemplate == VICUS::INVALID_ID)
//			continue;
//		// lookup zone template in DB
//		const VICUS::AcousticTemplate * at = db_at[room->m_idAcousticTemplate];
//		// Note: might be a nullptr if id is invalid
//		m_acousticTemplateAssignments[at].push_back(room);
//	}


	// now put the data of the map into the table
	int currentRow = m_ui->tableWidgetAcousticTemplates->currentRow();
	m_ui->tableWidgetAcousticTemplates->blockSignals(true);
	m_ui->tableWidgetAcousticTemplates->clearContents();
	int row=0;

	// loads all the templates from db and put them in the table
	for (std::map<unsigned int, VICUS::AcousticTemplate>::const_iterator
		 it = db_at.begin(); it != db_at.end(); ++it, ++row)
	{
		m_ui->tableWidgetAcousticTemplates->setRowCount(row + 1);
		QTableWidgetItem * item = new QTableWidgetItem();
		item->setBackground(it->second.m_color);
		item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		m_ui->tableWidgetAcousticTemplates->setItem(row, 0, item);

		item = new QTableWidgetItem();
		item->setText(QtExt::MultiLangString2QString(it->second.m_displayName) );
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetAcousticTemplates->setItem(row, 1, item);

		item = new QTableWidgetItem();
		item->setText(QtExt::MultiLangString2QString(it->second.m_notes) );
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetAcousticTemplates->setItem(row, 2, item);
	}
	m_ui->tableWidgetAcousticTemplates->blockSignals(false);
	m_ui->tableWidgetAcousticTemplates->selectRow(std::min(currentRow, m_ui->tableWidgetAcousticTemplates->rowCount()-1));

	// process all selected rooms and determine which zone template they have assigned
	std::vector<const VICUS::Room*> rooms;
	project().selectedRooms(rooms);
	if (rooms.empty()) {
		m_ui->labelSelectedAcousticTemplate->setText("");
		m_ui->groupBoxSelectedRooms->setEnabled(false);
	}
	else {
		m_ui->groupBoxSelectedRooms->setEnabled(true);
	}

	// update push button for selected zone assignement
	m_ui->pushButtonAssignAcousticTemplate->setEnabled(!rooms.empty());
	m_ui->pushButtonAssignAcousticTemplate->setEnabled(m_ui->tableWidgetAcousticTemplates->rowCount() == 1);

	const SVDatabase & db = SVSettings::instance().m_db;

	// update selection-related info
	std::set<const VICUS::AcousticTemplate *> selectedAcousticTemplate;
	// loop over all selected rooms and store pointer to assigned zone template
	for (const VICUS::Room* r : rooms) {
		//TODO Anton laut dieser implementierung wird das falsch angezeigt wenn ein Room ohne Template selektiert wird und ein Room mit einem Template+
		// sieht so aus als ob beide Rooms das eine Template besitzen, obwohl einer gar keins hat
		if (r->m_idAcousticTemplate != VICUS::INVALID_ID) {
			const VICUS::AcousticTemplate * at= db.m_acousticTemplates[r->m_idAcousticTemplate];
			selectedAcousticTemplate.insert(at);
		}
	}
	if (selectedAcousticTemplate.empty()) {
		m_ui->labelSelectedAcousticTemplate->setText(tr("None"));
	}
	else if (selectedAcousticTemplate.size() == 1) {
		const VICUS::AcousticTemplate * at = *selectedAcousticTemplate.begin();
		// special handling: exactly one room with invalid zone template ID is selected
		if (at == nullptr)
			m_ui->labelSelectedAcousticTemplate->setText(tr("Acoustic template with invalid/unknown ID"));
		else // otherwise show info about the selected zone template
			m_ui->labelSelectedAcousticTemplate->setText(tr("%1 [%2]")
			   .arg(QtExt::MultiLangString2QString(at->m_displayName)).arg(at->m_id) );
	}
	else {
		m_ui->labelSelectedAcousticTemplate->setText(tr("%1 different templates")
		   .arg(selectedAcousticTemplate.size()));
	}
	// update table related button states
	on_tableWidgetAcousticTemplates_itemSelectionChanged();
}

void SVPropBuildingAcousticTemplatesWidget::on_tableWidgetAcousticTemplates_itemSelectionChanged() {
	// the assign-from-table button is only available when there is at least one surface selected
	m_ui->pushButtonAssignAcousticTemplate->setEnabled(currentlySelectedAcousticTemplate() != nullptr);
}


const VICUS::AcousticTemplate * SVPropBuildingAcousticTemplatesWidget::currentlySelectedAcousticTemplate() const {
	// check if selected "template" is actually missing
	int r = m_ui->tableWidgetAcousticTemplates->currentRow();
	if (r == -1)
		return nullptr;
	const VICUS::Database<VICUS::AcousticTemplate> & db_at = SVSettings::instance().m_db.m_acousticTemplates;
	std::map<unsigned int, VICUS::AcousticTemplate>::const_iterator it = db_at.begin();
	std::advance(it, r);
	return &(it->second);
}




void SVPropBuildingAcousticTemplatesWidget::on_pushButtonAssignAcousticTemplate_clicked() {

	// find out which component is selected in table
	// get currently selected acoustic template
	const VICUS::AcousticTemplate * at = currentlySelectedAcousticTemplate();

	// if not a valid template, do nothing here
	if (at == nullptr)
		return;

	// get all visible _and_ selected "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, true, true);

	std::vector<unsigned int> modifiedRoomIDs; // unique IDs!!!
	// loop over all rooms and store acoustic template associations
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		modifiedRoomIDs.push_back(room->m_id);
	}
	// now create an undo action for modifying zone template assignments
	SVUndoModifyRoomAcousticTemplateAssociation * undo = new SVUndoModifyRoomAcousticTemplateAssociation(
				tr("Assigned acoustic template"),
				modifiedRoomIDs, at->m_id);
	undo->push();
}

