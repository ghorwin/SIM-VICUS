#include "SVPropBuildingZoneProperty.h"
#include "ui_SVPropBuildingZoneProperty.h"

#include <SVConversions.h>

#include "SVStyle.h"
#include "SVPropZonePropertyDelegate.h"
#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyProject.h"
#include "SVUndoModifyBuildingLevel.h"
#include "SVUndoModifyRoom.h"
#include "SVUndoModifyBuildingTopology.h"
#include "SVUndoModifyProject.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVZoneSelectionDialog.h"

#include <QSortFilterProxyModel>
#include <QProgressDialog>

#include <RC_VicusClipping.h>
#include <RC_ClippingSurface.h>

class CalculationProgress : public Notification {
public:
	void notify() override {}
	void notify(double percentage) override;

	char				pad[7]; // fix padding, silences compiler warning
	QProgressDialog		*m_dlg = nullptr;
};

void CalculationProgress::notify(double percentage) {
	m_dlg->setValue((int)(m_dlg->maximum() * percentage));
	qApp->processEvents();
	if (m_dlg->wasCanceled())
		m_aborted = true;
}

SVPropBuildingZoneProperty::SVPropBuildingZoneProperty(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingZoneProperty),
	m_zonePropertiesTableModel(new SVPropBuildingZonePropertyTableModel(this))
{
	m_ui->setupUi(this);
	m_ui->mainGridLayout->setMargin(0);

	// create proxy model and link with table model and view
	m_zonePropertiesProxyModel = new SVPropBuildingZonePropertyTableProxyModel(this);
	m_zonePropertiesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_zonePropertiesProxyModel->setSourceModel(m_zonePropertiesTableModel);

	SVStyle::formatDatabaseTableView(m_ui->tableViewZones);
	m_ui->tableViewZones->sortByColumn(1, Qt::SortOrder::AscendingOrder);
	m_ui->tableViewZones->setSortingEnabled(true);
//	m_ui->tableViewZones->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
//	m_ui->tableViewZones->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
//	m_ui->tableViewZones->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//	m_ui->tableViewZones->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//	m_ui->tableViewZones->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
	m_ui->tableViewZones->horizontalHeader()->resizeSection(0,100);
	m_ui->tableViewZones->horizontalHeader()->resizeSection(1,150);
	m_ui->tableViewZones->horizontalHeader()->resizeSection(2,100);
	m_ui->tableViewZones->horizontalHeader()->resizeSection(3,100);
	m_ui->tableViewZones->horizontalHeader()->resizeSection(4,100);
	m_ui->tableViewZones->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableViewZones->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->tableViewZones->setSelectionBehavior(QAbstractItemView::SelectRows); // since we want to edit single cells

	m_ui->tableViewZones->setModel(m_zonePropertiesProxyModel);

	// Mind: parent of the item delegate must be its widget!
	m_ui->tableViewZones->setItemDelegate(new SVPropZonePropertyDelegate(m_ui->tableViewZones));

	m_ui->pushButtonAssignSurface->setEnabled(false);

	// connections
	connect(m_ui->tableViewZones->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &SVPropBuildingZoneProperty::updateTableView);
	connect(m_ui->tableViewZones->model(), &QAbstractItemModel::layoutChanged,
			this, &SVPropBuildingZoneProperty::updateTableView);
}


SVPropBuildingZoneProperty::~SVPropBuildingZoneProperty() {
	delete m_ui;
}


void SVPropBuildingZoneProperty::updateUi() {


	// first update building and building level

	m_ui->comboBoxBuildingFilter->blockSignals(true);
	m_ui->comboBoxBuildingLevelFilter->blockSignals(true);

	unsigned int currentBuilding = m_ui->comboBoxBuildingFilter->currentData().toUInt();	// unique ID of currently selected building
	unsigned int currentBuildingLevel = m_ui->comboBoxBuildingLevelFilter->currentData().toUInt();	// unique ID of currently selected building
	int selectedBuildingIndex = -1 ;
	int selectedBuildingLevelIndex = -1 ;
	unsigned int currentBuildingVectorIdx = VICUS::INVALID_ID;

	m_ui->comboBoxBuildingFilter->clear();
	m_ui->comboBoxBuildingLevelFilter->clear();

	m_zonePropertiesTableModel->reset();

	// set all building in combo box building
	m_ui->comboBoxBuildingFilter->addItem(tr("All buildings"), VICUS::INVALID_ID);
	for(unsigned int i=0; i<project().m_buildings.size(); ++i){
		const VICUS::Building & b = project().m_buildings[i];
		if(currentBuilding == b.m_id){
			selectedBuildingIndex = m_ui->comboBoxBuildingFilter->count();
			currentBuildingVectorIdx = i;
		}
		m_ui->comboBoxBuildingFilter->addItem(b.m_displayName, b.m_id);
	}

	if (selectedBuildingIndex == -1) {
		m_ui->comboBoxBuildingLevelFilter->setCurrentIndex(0);
		m_ui->comboBoxBuildingFilter->setCurrentIndex(0);
	}
	else
		m_ui->comboBoxBuildingFilter->setCurrentIndex(selectedBuildingIndex);

	m_ui->comboBoxBuildingLevelFilter->addItem(tr("All building levels"), VICUS::INVALID_ID);
	if(selectedBuildingIndex == -1){
//		for(const VICUS::Building & b : project().m_buildings){
//			for(const VICUS::BuildingLevel & bl : b.m_buildingLevels){
//				if(currentBuildingLevel == bl.m_id)
//					selectedBuildingLevelIndex = m_ui->comboBoxBuildingLevelFilter->count();
//				m_ui->comboBoxBuildingLevelFilter->addItem(bl.m_displayName, bl.m_id);
//			}
//		}
		selectedBuildingLevelIndex = 0; // I guess that is better since we want to have an hierarchical selection
	}
	else{
		for(unsigned int i=0; i<project().m_buildings[currentBuildingVectorIdx].m_buildingLevels.size(); ++i){
			const VICUS::BuildingLevel & bl = project().m_buildings[currentBuildingVectorIdx].m_buildingLevels[i];
			if(currentBuildingLevel == bl.m_id){
				selectedBuildingLevelIndex = m_ui->comboBoxBuildingLevelFilter->count();
			}
			m_ui->comboBoxBuildingLevelFilter->addItem(bl.m_displayName, bl.m_id);
		}
	}

	if(selectedBuildingLevelIndex == -1)
		m_ui->comboBoxBuildingLevelFilter->setCurrentIndex(0);
	else
		m_ui->comboBoxBuildingLevelFilter->setCurrentIndex(selectedBuildingLevelIndex);

	m_ui->comboBoxBuildingLevelFilter->blockSignals(false);
	m_ui->comboBoxBuildingFilter->blockSignals(false);

	// set rooms selection in table model
	m_zonePropertiesTableModel->updateBuildingLevelIndex(selectedBuildingIndex, selectedBuildingLevelIndex);

	m_ui->comboBoxBuildingLevelFilter->setEnabled(selectedBuildingIndex != -1);

	// update "Assign surface" button state
	updateTableView();
}


void SVPropBuildingZoneProperty::on_comboBoxBuildingFilter_currentIndexChanged(int /*index*/){
	updateUi();
}



void SVPropBuildingZoneProperty::on_comboBoxBuildingLevelFilter_currentIndexChanged(int /*index*/) {
	updateUi();
}

void SVPropBuildingZoneProperty::on_tableViewZones_doubleClicked(const QModelIndex &index) {
	if (index.isValid())
		m_ui->tableViewZones->closePersistentEditor(index);

	if (index.column() == 1)
		m_zonePropertiesTableModel->showZoneInformation(index);
}


void SVPropBuildingZoneProperty::on_pushButtonAssignSurface_clicked() {

	// no valid index is chosen
	if(!m_selectedProxyIndex.isValid())
		return;

	// retrieve name from column 2
	QModelIndex proxyIndex = m_zonePropertiesProxyModel->index(m_selectedProxyIndex.row(), 1);
	// error in table model creation
	Q_ASSERT(proxyIndex.isValid());

	QModelIndex index = m_zonePropertiesProxyModel->mapToSource(proxyIndex);

	QString messageBoxText;
	bool success = m_zonePropertiesTableModel->assignSurfaces(index, messageBoxText);

	QString newRoomName = m_zonePropertiesTableModel->data(index, Qt::DisplayRole).toString();

	if(success) {
		QMessageBox::information(this, QString("Assigning surfaces to room '%1'").arg(newRoomName), messageBoxText );
	}
	else {
		QMessageBox::information(this, QString("Assigning surfaces to room '%1'").arg(newRoomName), QString("All surfaces were already assigned to room '%1'").arg(newRoomName) );
	}
}


void SVPropBuildingZoneProperty::on_pushButtonFloorAreaSelectedRooms_clicked() {

	// empty selection
	if(m_selectedProxyIndexes.empty())
		return;

	// translate proxy selection into table model selection
	QItemSelection selection(m_selectedProxyIndexes.first(), m_selectedProxyIndexes.last());

	QModelIndexList indexes = m_zonePropertiesProxyModel->mapSelectionToSource(selection).indexes();
	Q_ASSERT(!indexes.empty());

	// Create progress dialog
	QProgressDialog progressDialog(tr("Calculate floor area of selected rooms."), tr("Abort"), 0, 100, this);
	progressDialog.setValue(0);
	progressDialog.setMinimumDuration(1000);
//	progressDialog.show();

	// ProgressBar initialization
	CalculationProgress progressNotifyer;
	progressNotifyer.m_dlg = &progressDialog;

	// perform calculation of room floor areas inside table model
	m_zonePropertiesTableModel->calulateFloorArea(&progressNotifyer, indexes);

	if (progressNotifyer.m_aborted) {
		QMessageBox::information(this, QString(), tr("Calculation of floor areas was aborted."));
		return;
	}

	progressDialog.hide();
}


void SVPropBuildingZoneProperty::on_pushButtonVolumeSelectedRooms_clicked() {

	// empty selection
	if(m_selectedProxyIndexes.empty())
		return;

	// translate proxy selection into table model selection
	QItemSelection selection(m_selectedProxyIndexes.first(), m_selectedProxyIndexes.last());

	QModelIndexList indexes = m_zonePropertiesProxyModel->mapSelectionToSource(selection).indexes();
	Q_ASSERT(!indexes.empty());

	// Create progress dialog
	QProgressDialog progressDialog(tr("Calculate volumes of selected rooms."), tr("Abort"), 0, 100, this);
	progressDialog.setValue(0);
	progressDialog.setMinimumDuration(1000);
//	progressDialog.show();

	// ProgressBar initialization
	CalculationProgress progressNotifyer;
	progressNotifyer.m_dlg = &progressDialog;

	// perform calculation of room floor areas inside table model
	m_zonePropertiesTableModel->calulateVolume(&progressNotifyer, indexes);

	if (progressNotifyer.m_aborted) {
		QMessageBox::information(this, QString(), tr("Calculation of volumes was aborted."));
		return;
	}

	progressDialog.hide();
}


void SVPropBuildingZoneProperty::on_pushButtonFloorAreaAllRooms_clicked() {

	QModelIndexList indexes;
	// choose all rooms in proxy model
	for(int row=0; row<m_zonePropertiesProxyModel->rowCount(); ++row) {
		QModelIndex index = m_zonePropertiesProxyModel->index(row,0);
		indexes.push_back(m_zonePropertiesProxyModel->mapToSource(index));
	}
	// empty
	if(indexes.empty())
		return;

	// Create progress dialog
	QProgressDialog progressDialog(tr("Calculate floor area of all rooms."), tr("Abort"), 0, 100, this);
	progressDialog.setValue(0);
	progressDialog.setMinimumDuration(1000);
//	progressDialog.show();

	// ProgressBar initialization
	CalculationProgress progressNotifyer;
	progressNotifyer.m_dlg = &progressDialog;

	// perform calculation of room floor areas inside table model
	m_zonePropertiesTableModel->calulateFloorArea(&progressNotifyer, indexes);

	if (progressNotifyer.m_aborted) {
		QMessageBox::information(this, QString(), tr("Calculation of floor areas was aborted."));
		return;
	}

	progressDialog.hide();
}


void SVPropBuildingZoneProperty::on_pushButtonVolumeAllRooms_clicked() {

	QModelIndexList indexes;
	// choose all rooms in proxy model
	for(int row=0; row<m_zonePropertiesProxyModel->rowCount(); ++row) {
		QModelIndex index = m_zonePropertiesProxyModel->index(row,0);
		indexes.push_back(m_zonePropertiesProxyModel->mapToSource(index));
	}
	// empty
	if(indexes.empty())
		return;

	// Create progress dialog
	QProgressDialog progressDialog(tr("Calculate volumes of all rooms."), tr("Abort"), 0, 100, this);
	progressDialog.setValue(0);
	progressDialog.setMinimumDuration(1000);
//	progressDialog.show();

	// ProgressBar initialization
	CalculationProgress progressNotifyer;
	progressNotifyer.m_dlg = &progressDialog;

	// perform calculation of room volumes inside table model
	m_zonePropertiesTableModel->calulateVolume(&progressNotifyer, indexes);

	if (progressNotifyer.m_aborted) {
		QMessageBox::information(this, QString(), tr("Calculation of volumes was aborted."));
		return;
	}

	progressDialog.hide();
}


void SVPropBuildingZoneProperty::updateTableView() {
//	qDebug() << "bei jedem Klick in die Tabelle mit Änderung der Selection sollte das hier ausgegeben werden, sonst ist irgendwo blockSignals() noch aktiv";

	m_selectedProxyIndex = QModelIndex();
	// TODO Dirk: pushButtonAssignSurface darf nur aktiv sein, wenn:
	//            - exakt ein Raum/eine Zeile ausgewählt ist   (m_ui->tableViewZones->selectionModel()->selectedRows() == 1)
	//            - mindestens eine Fläche ausgewählt ist, die *nicht* bereits dem gewählten Raum gehört

	m_selectedProxyIndexes = m_ui->tableViewZones->selectionModel()->selectedRows(1);
	m_ui->pushButtonAssignSurface->setEnabled(true);

	std::vector<const VICUS::Surface*> surfs;
	project().selectedSurfaces(surfs, VICUS::Project::SG_Building);

	m_ui->pushButtonFloorAreaSelectedRooms->setEnabled(m_selectedProxyIndexes.size() > 0);
	m_ui->pushButtonVolumeSelectedRooms->setEnabled(m_selectedProxyIndexes.size() > 0);

	if (m_selectedProxyIndexes.empty()) {
		// no selection for calculating volume and floor area
		m_ui->pushButtonFloorAreaSelectedRooms->setEnabled(false);
		m_ui->pushButtonVolumeSelectedRooms->setEnabled(false);
		m_ui->pushButtonAssignSurface->setEnabled(false);
		return;
	}

	// we have a valid selection for calculation of zone floor areas and volumes
	m_ui->pushButtonFloorAreaSelectedRooms->setEnabled(true);
	m_ui->pushButtonVolumeSelectedRooms->setEnabled(true);

	// for surface assignment only a single selection is allowed
	if (m_selectedProxyIndexes.size() != 1) {
		m_ui->pushButtonAssignSurface->setEnabled(false);
		return;
	}

	// get id of single selection
	unsigned int id = (unsigned int) m_zonePropertiesProxyModel->data(m_selectedProxyIndexes[0], Qt::UserRole).toInt();

	bool isPartOfRoom = true;
	for(const VICUS::Surface *s : surfs) {
		if (s->m_parent->m_id != id)
			isPartOfRoom = false;
	}

	if(isPartOfRoom) {
		// set an invalid index
		m_ui->pushButtonAssignSurface->setEnabled(false);
		return;
	}

	Q_ASSERT(dynamic_cast<const VICUS::Room*>(project().objectById(id)) != nullptr);

	// allow surface assignment
	m_ui->pushButtonAssignSurface->setEnabled(true);

	m_selectedProxyIndex = m_selectedProxyIndexes[0];
}


void SVPropBuildingZoneProperty::on_lineEditNameFilter_textChanged(const QString &name) {
	m_zonePropertiesProxyModel->setNameFilter(name);
}

