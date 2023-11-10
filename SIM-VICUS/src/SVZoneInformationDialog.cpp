#include "SVZoneInformationDialog.h"
#include "ui_SVZoneInformationDialog.h"

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVUndoModifyRoom.h"

SVZoneInformationDialog::SVZoneInformationDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVZoneInformationDialog)
{
	m_ui->setupUi(this);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSurfaceList);
	m_ui->tableWidgetSurfaceList->setColumnCount(NumCol);

	QStringList headers;
	headers << tr("ID")
			<< tr("Name")
			<< tr("Area [m2]")
			<< tr("Component")
			<< tr("Construction")
			<< tr("Boundary A")
			<< tr("Boundary B");
	m_ui->tableWidgetSurfaceList->setHorizontalHeaderLabels(headers);

	m_ui->tableWidgetSurfaceList->setSortingEnabled(false);
	m_ui->tableWidgetSurfaceList->horizontalHeader()->setStretchLastSection(true);

	connect(m_ui->lineEditHeatCapacity, &QtExt::ValidatingLineEdit::editingFinishedSuccessfully,
			this, &SVZoneInformationDialog::on_lineEditHeatCapacityFinishedSuccessfully);

	connect(m_ui->lineEditArea, &QtExt::ValidatingLineEdit::editingFinishedSuccessfully,
			this, &SVZoneInformationDialog::on_lineEditAreaFinishedSuccessfully);

	connect(m_ui->lineEditVolume, &QtExt::ValidatingLineEdit::editingFinishedSuccessfully,
			this, &SVZoneInformationDialog::on_lineEditVolumeFinishedSuccessfully);

}

SVZoneInformationDialog::~SVZoneInformationDialog() {
	delete m_ui;
}

void SVZoneInformationDialog::showZoneInformation(const VICUS::Project &prj, unsigned int zoneId) {

	// create and configure
	const VICUS::Object *obj = prj.objectById(zoneId);
	const VICUS::Room *room = dynamic_cast<const VICUS::Room *>(obj);

	if (room == nullptr)
		return;

	m_currentRoom = const_cast<VICUS::Room *>(room);
	const SVDatabase &db = SVSettings::instance().m_db;

	if (room->m_idZoneTemplate != VICUS::INVALID_ID) {
		const VICUS::ZoneTemplate *zt = db.m_zoneTemplates[room->m_idZoneTemplate];
		Q_ASSERT(zt != nullptr);

		m_ui->labelZoneTemplate->setText(QString::fromStdString(zt->m_displayName.encodedString()));
	}
	else
		m_ui->labelZoneTemplate->setText(tr("No template set"));

	if (room->m_idSoundProtectionTemplate != VICUS::INVALID_ID) {
		const VICUS::AcousticSoundProtectionTemplate *aspt = db.m_acousticSoundProtectionTemplates[room->m_idSoundProtectionTemplate];
		Q_ASSERT(aspt != nullptr);

		m_ui->labelSoundprotectionTemplate->setText(QString::fromStdString(aspt->m_displayName.encodedString()));
	}
	else
		m_ui->labelSoundprotectionTemplate->setText(tr("No template set"));

	if (room->m_idAcousticTemplate != VICUS::INVALID_ID) {
		const VICUS::AcousticTemplate *at = db.m_acousticTemplates[room->m_idAcousticTemplate];
		Q_ASSERT(at != nullptr);

		m_ui->labelAcousticTemplate->setText(QString::fromStdString(at->m_displayName.encodedString()));
	}
	else
		m_ui->labelAcousticTemplate->setText(tr("No template set"));


	if (room->m_idAcousticBuildingType != VICUS::INVALID_ID) {
		const VICUS::AcousticBuildingTemplate *abt = db.m_acousticBuildingTemplates[room->m_idAcousticBuildingType];
		Q_ASSERT(abt != nullptr);

		m_ui->labelBuildingType->setText(QString::fromStdString(abt->m_displayName.encodedString()));
	}
	else
		m_ui->labelBuildingType->setText(tr("No template set"));

	m_ui->lineEditArea->setText(QString("%1").arg(room->m_para[VICUS::Room::P_Area].value, 0, 'g', 3));
	m_ui->lineEditVolume->setText(QString("%1").arg(room->m_para[VICUS::Room::P_Volume].value, 0, 'g', 3));
	m_ui->lineEditHeatCapacity->setText(QString("%1").arg(room->m_para[VICUS::Room::P_HeatCapacity].value, 0, 'g', 3));

	m_ui->labelName->setText(room->m_displayName);

	m_ui->tableWidgetSurfaceList->setRowCount(room->m_surfaces.size());
	for (unsigned int i = 0; i < room->m_surfaces.size(); ++i) {
		const VICUS::Surface &s = room->m_surfaces[i];

		QTableWidgetItem *item = new QTableWidgetItem(s.m_displayName);
		m_ui->tableWidgetSurfaceList->setItem(i, ColSurfaceName, item);

		item = new QTableWidgetItem(QString::number(s.m_id));
		m_ui->tableWidgetSurfaceList->setItem(i, ColID, item);

		item = new QTableWidgetItem(QString::number(s.geometry().area()));
		m_ui->tableWidgetSurfaceList->setItem(i, ColArea, item);

		std::string componentName = "-";
		const VICUS::Component *comp = nullptr;
		if (s.m_componentInstance != nullptr) {
			comp = db.m_components[s.m_componentInstance->m_idComponent];
			componentName = comp->m_displayName.encodedString();
		}

		item = new QTableWidgetItem(QString::fromStdString(componentName));
		m_ui->tableWidgetSurfaceList->setItem(i, ColComponentName, item);

		std::string constructionName = "-";
		if (comp != nullptr)
			constructionName = db.m_constructions[comp->m_idConstruction]->m_displayName.encodedString();

		item = new QTableWidgetItem(QString::fromStdString(constructionName));
		m_ui->tableWidgetSurfaceList->setItem(i, ColConstructionName, item);

		std::string boundA = "-";
		if (comp != nullptr && comp->m_idSideABoundaryCondition != VICUS::INVALID_ID) {
			boundA = db.m_boundaryConditions[comp->m_idSideABoundaryCondition]->m_displayName.encodedString();
		}
		item = new QTableWidgetItem(QString::fromStdString(boundA));
		m_ui->tableWidgetSurfaceList->setItem(i, ColBoundaryConditionSiteA, item);

		std::string boundB = "-";
		if (comp != nullptr && comp->m_idSideBBoundaryCondition != VICUS::INVALID_ID) {
			boundB = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition]->m_displayName.encodedString();
		}
		item = new QTableWidgetItem(QString::fromStdString(boundB));
		m_ui->tableWidgetSurfaceList->setItem(i, ColBoundaryConditionSiteB, item);
	}

	m_ui->tableWidgetSurfaceList->resizeColumnsToContents();

	// Run dialog
	exec();
}

void SVZoneInformationDialog::on_lineEditHeatCapacityFinishedSuccessfully() {

	if (!m_ui->lineEditHeatCapacity->isValid() || m_currentRoom == nullptr)
		return;

	VICUS::Room newRoom(*m_currentRoom);

	const VICUS::BuildingLevel *bl = dynamic_cast<const VICUS::BuildingLevel *>(newRoom.m_parent);
	Q_ASSERT(bl != nullptr);

	const VICUS::Building *b = dynamic_cast<const VICUS::Building *>(bl->m_parent);
	Q_ASSERT(b != nullptr);

	double val = m_ui->lineEditHeatCapacity->value();
	VICUS::KeywordList::setParameter(newRoom.m_para,"Room::para_t", VICUS::Room::P_HeatCapacity, val);

	SVUndoModifyRoom *undo = new SVUndoModifyRoom(tr("Updated heat capacity of room"), newRoom);
	undo->push();

	qDebug() << " set to " << m_currentRoom->m_para[VICUS::Room::P_HeatCapacity].value;

}


void SVZoneInformationDialog::on_lineEditAreaFinishedSuccessfully() {

	if (!m_ui->lineEditArea->isValid() || m_currentRoom == nullptr)
		return;

	VICUS::Room newRoom(*m_currentRoom);

	const VICUS::BuildingLevel *bl = dynamic_cast<const VICUS::BuildingLevel *>(newRoom.m_parent);
	Q_ASSERT(bl != nullptr);

	const VICUS::Building *b = dynamic_cast<const VICUS::Building *>(bl->m_parent);
	Q_ASSERT(b != nullptr);

	double val = m_ui->lineEditArea->value();
	VICUS::KeywordList::setParameter(newRoom.m_para,"Room::para_t", VICUS::Room::P_Area, val);

	SVUndoModifyRoom *undo = new SVUndoModifyRoom(tr("Updated area of room"), newRoom);
	undo->push();

	qDebug() << "Area set to " << m_currentRoom->m_para[VICUS::Room::P_Area].value;
}


void SVZoneInformationDialog::on_lineEditVolumeFinishedSuccessfully() {

	if (!m_ui->lineEditVolume->isValid() || m_currentRoom == nullptr)
		return;

	VICUS::Room newRoom(*m_currentRoom);

	const VICUS::BuildingLevel *bl = dynamic_cast<const VICUS::BuildingLevel *>(newRoom.m_parent);
	Q_ASSERT(bl != nullptr);

	const VICUS::Building *b = dynamic_cast<const VICUS::Building *>(bl->m_parent);
	Q_ASSERT(b != nullptr);

	double val = m_ui->lineEditVolume->value();
	VICUS::KeywordList::setParameter(newRoom.m_para,"Room::para_t", VICUS::Room::P_Volume, val);

	SVUndoModifyRoom *undo = new SVUndoModifyRoom(tr("Updated volume of room"), newRoom);
	undo->push();

	qDebug() << "Volume set to " << m_currentRoom->m_para[VICUS::Room::P_Volume].value;
}
