#include "SVZoneInformationDialog.h"
#include "ui_SVZoneInformationDialog.h"

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVUndoModifyRoom.h"

SVZoneInformationDialog::SVZoneInformationDialog(QWidget *parent, VICUS::Room *room) :
	QDialog(parent),
	m_ui(new Ui::SVZoneInformationDialog), m_room(room)
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
}

SVZoneInformationDialog::~SVZoneInformationDialog() {
	delete m_ui;
}

void SVZoneInformationDialog::showZoneInformation(const QString &title, const VICUS::Project &prj, unsigned int zoneId) {

	// create and configure dialog


	const VICUS::Object *obj = prj.objectById(zoneId);
	const VICUS::Room *room = dynamic_cast<const VICUS::Room *>(obj);

	if (room == nullptr)
		return;

	SVZoneInformationDialog dlg(nullptr, const_cast<VICUS::Room *>(room)); // top level

	const SVDatabase &db = SVSettings::instance().m_db;

	if (room->m_idZoneTemplate != VICUS::INVALID_ID) {
		const VICUS::ZoneTemplate *zt = db.m_zoneTemplates[room->m_idZoneTemplate];
		Q_ASSERT(zt != nullptr);

		dlg.m_ui->labelZoneTemplate->setText(QString::fromStdString(zt->m_displayName.encodedString()));
	}
	else
		dlg.m_ui->labelZoneTemplate->setText(tr("No template set"));

	if (room->m_idSoundProtectionTemplate != VICUS::INVALID_ID) {
		const VICUS::AcousticSoundProtectionTemplate *aspt = db.m_acousticSoundProtectionTemplates[room->m_idSoundProtectionTemplate];
		Q_ASSERT(aspt != nullptr);

		dlg.m_ui->labelSoundprotectionTemplate->setText(QString::fromStdString(aspt->m_displayName.encodedString()));
	}
	else
		dlg.m_ui->labelSoundprotectionTemplate->setText(tr("No template set"));

	if (room->m_idAcousticTemplate != VICUS::INVALID_ID) {
		const VICUS::AcousticTemplate *at = db.m_acousticTemplates[room->m_idAcousticTemplate];
		Q_ASSERT(at != nullptr);

		dlg.m_ui->labelAcousticTemplate->setText(QString::fromStdString(at->m_displayName.encodedString()));
	}
	else
		dlg.m_ui->labelAcousticTemplate->setText(tr("No template set"));


	if (room->m_idAcousticBuildingType != VICUS::INVALID_ID) {
		const VICUS::AcousticBuildingTemplate *abt = db.m_acousticBuildingTemplates[room->m_idAcousticBuildingType];
		Q_ASSERT(abt != nullptr);

		dlg.m_ui->labelBuildingType->setText(QString::fromStdString(abt->m_displayName.encodedString()));
	}
	else
		dlg.m_ui->labelBuildingType->setText(tr("No template set"));

	dlg.m_ui->labelArea->setText(QString("%1 m<sup>2</sup>").arg(room->m_para[VICUS::Room::P_Area].value));
	dlg.m_ui->labelVolume->setText(QString("%1 m<sup>3</sup>").arg(room->m_para[VICUS::Room::P_Volume].value));
	dlg.m_ui->heatCapacityLineEdit->setText(QString("%1").arg(room->m_para[VICUS::Room::P_HeatCapacity].value));
	qDebug() << "Heat Capacity set to: " << room->m_para[VICUS::Room::P_HeatCapacity].value;

	dlg.m_ui->labelName->setText(room->m_displayName);

	dlg.m_ui->tableWidgetSurfaceList->setRowCount(room->m_surfaces.size());
	for (unsigned int i = 0; i < room->m_surfaces.size(); ++i) {
		const VICUS::Surface &s = room->m_surfaces[i];

		QTableWidgetItem *item = new QTableWidgetItem(s.m_displayName);
		dlg.m_ui->tableWidgetSurfaceList->setItem(i, ColSurfaceName, item);

		item = new QTableWidgetItem(QString::number(s.m_id));
		dlg.m_ui->tableWidgetSurfaceList->setItem(i, ColID, item);

		item = new QTableWidgetItem(QString::number(s.geometry().area()));
		dlg.m_ui->tableWidgetSurfaceList->setItem(i, ColArea, item);

		std::string componentName = "-";
		const VICUS::Component *comp = nullptr;
		if (s.m_componentInstance != nullptr) {
			comp = db.m_components[s.m_componentInstance->m_idComponent];
			componentName = comp->m_displayName.encodedString();
		}

		item = new QTableWidgetItem(QString::fromStdString(componentName));
		dlg.m_ui->tableWidgetSurfaceList->setItem(i, ColComponentName, item);

		std::string constructionName = "-";
		if (comp != nullptr)
			constructionName = db.m_constructions[comp->m_idConstruction]->m_displayName.encodedString();

		item = new QTableWidgetItem(QString::fromStdString(constructionName));
		dlg.m_ui->tableWidgetSurfaceList->setItem(i, ColConstructionName, item);

		std::string boundA = "-";
		if (comp != nullptr && comp->m_idSideABoundaryCondition != VICUS::INVALID_ID) {
			boundA = db.m_boundaryConditions[comp->m_idSideABoundaryCondition]->m_displayName.encodedString();
		}
		item = new QTableWidgetItem(QString::fromStdString(boundA));
		dlg.m_ui->tableWidgetSurfaceList->setItem(i, ColBoundaryConditionSiteA, item);

		std::string boundB = "-";
		if (comp != nullptr && comp->m_idSideBBoundaryCondition != VICUS::INVALID_ID) {
			boundB = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition]->m_displayName.encodedString();
		}
		item = new QTableWidgetItem(QString::fromStdString(boundB));
		dlg.m_ui->tableWidgetSurfaceList->setItem(i, ColBoundaryConditionSiteB, item);
	}

	dlg.m_ui->tableWidgetSurfaceList->resizeColumnsToContents();

	if (title.isEmpty())
		dlg.setWindowTitle(qApp->applicationName());
	else
		dlg.setWindowTitle(title);

	dlg.exec();

	return;

}

void SVZoneInformationDialog::on_heatCapacityLineEdit_editingFinished()
{
	if (!m_ui->heatCapacityLineEdit->isValid() || m_room == nullptr)
		return;

	VICUS::Room newRoom = *m_room;

	const VICUS::BuildingLevel *bl = dynamic_cast<const VICUS::BuildingLevel *>(newRoom.m_parent);
	Q_ASSERT(bl != nullptr);

	const VICUS::Building *b = dynamic_cast<const VICUS::Building *>(bl->m_parent);
	Q_ASSERT(b != nullptr);

	double val = m_ui->heatCapacityLineEdit->value();
	VICUS::KeywordList::setParameter(newRoom.m_para,"Room::para_t", VICUS::Room::P_HeatCapacity, val);

	SVUndoModifyRoom *undo = new SVUndoModifyRoom(tr("Updated heat capacity of room"), newRoom, b->m_id, bl->m_id, m_room->m_id);
	undo->push();

	qDebug() << "Heat capacity set to " << m_room->m_para[VICUS::Room::P_HeatCapacity].value;
}
