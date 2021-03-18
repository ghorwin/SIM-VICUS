#include "SVDBZoneTemplateEditWidget.h"
#include "ui_SVDBZoneTemplateEditWidget.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <VICUS_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include "SVDBZoneTemplateTreeModel.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

SVDBZoneTemplateEditWidget::SVDBZoneTemplateEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBZoneTemplateEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(4);
	m_ui->verticalLayoutWidget->setMargin(0);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	// initial state is "nothing selected"
	updateInput(-1, -1, 0);
}


SVDBZoneTemplateEditWidget::~SVDBZoneTemplateEditWidget() {
	delete m_ui;
}


void SVDBZoneTemplateEditWidget::setup(SVDatabase * db, SVDBZoneTemplateTreeModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBZoneTemplateEditWidget::updateInput(int id, int subTemplateId, int subTemplateType) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_currentSubTemplateType = subTemplateType;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);

	m_ui->pushButtonAddPersonLoad->setEnabled(false);
	m_ui->pushButtonAddPersonLoad->setChecked(false);

	m_ui->pushButtonAddElectricLoad->setEnabled(false);
	m_ui->pushButtonAddElectricLoad->setChecked(false);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSubComponent->setText(QString());
		// hide widget with references
		m_ui->widget->setVisible(false);
		return;
	}

	VICUS::ZoneTemplate * item = const_cast<VICUS::ZoneTemplate*>(m_db->m_zoneTemplates[(unsigned int)id]);
	m_current = item;

	// now update the GUI controls
	m_ui->lineEditName->setString(m_current->m_displayName);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (m_current->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);

	// re-enable add buttons only when not yet set
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] == VICUS::INVALID_ID)
		m_ui->pushButtonAddPersonLoad->setEnabled(true);
	else {
		m_ui->pushButtonAddPersonLoad->setEnabled(false);
		m_ui->pushButtonAddPersonLoad->setChecked(true);
	}
	if (item->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] == VICUS::INVALID_ID)
		m_ui->pushButtonAddElectricLoad->setEnabled(true);
	else {
		m_ui->pushButtonAddElectricLoad->setEnabled(false);
		m_ui->pushButtonAddElectricLoad->setChecked(true);
	}

	// now the sub-template stuff
	if (subTemplateId == -1) {
		// hide widget with references
		m_ui->widget->setVisible(false);
		return;
	}
	// show widget with references
	m_ui->widget->setVisible(true);

	// determine which sub-template was selected
	switch ((VICUS::ZoneTemplate::SubTemplateType)subTemplateType) {
		case VICUS::ZoneTemplate::ST_IntLoadPerson: {
			m_ui->labelSubTemplate->setText(tr("Internal Loads - Person loads:"));
			// lookup corresponding dataset entry in database
			const VICUS::InternalLoad * iload = m_db->m_internalLoads[(unsigned int)subTemplateId];
			if (iload == nullptr) {
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			}
			else {
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(iload->m_displayName) );
			}
		} break;
		case VICUS::ZoneTemplate::ST_IntLoadEquipment:{
			m_ui->labelSubTemplate->setText(tr("Internal Loads - Equipment loads:"));
			// lookup corresponding dataset entry in database
			const VICUS::InternalLoad * iload = m_db->m_internalLoads[(unsigned int)subTemplateId];
			if (iload == nullptr) {
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			}
			else {
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(iload->m_displayName) );
			}
		} break;
		break;
		case VICUS::ZoneTemplate::ST_IntLoadLighting:
		break;
		case VICUS::ZoneTemplate::ST_IntLoadOther:
		break;
		case VICUS::ZoneTemplate::ST_ControlThermostat:
		break;

		case VICUS::ZoneTemplate::NUM_ST:
		break;
	}

}


void SVDBZoneTemplateEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_zoneTemplates.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBZoneTemplateEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		m_db->m_zoneTemplates.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}



void SVDBZoneTemplateEditWidget::on_toolButtonSelectSubComponent_clicked() {
	unsigned int id = VICUS::INVALID_ID;
	switch (m_currentSubTemplateType) {
		case VICUS::ZoneTemplate::ST_IntLoadPerson:
			id = SVMainWindow::instance().dbInternalLoadsPersonEditDialog()->select(m_current->m_idReferences[m_currentSubTemplateType]);
		break;
		case VICUS::ZoneTemplate::ST_IntLoadEquipment:
			id = SVMainWindow::instance().dbInternalLoadsElectricEquipmentEditDialog()->select(m_current->m_idReferences[m_currentSubTemplateType]);
		break;
	}
	if (id != VICUS::INVALID_ID) {
		// modify existing
		m_current->m_idReferences[m_currentSubTemplateType] = id;
		m_db->m_zoneTemplates.m_modified = true;
	}
	// we must assume that the name of the referenced sub-template has changed, so update controls accordingly
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	updateInput((int)m_current->m_id, (int)m_current->m_idReferences[m_currentSubTemplateType], m_currentSubTemplateType);
}


void SVDBZoneTemplateEditWidget::on_toolButtonRemoveSubComponent_clicked() {
	m_dbModel->deleteChildItem( m_dbModel->indexById(m_current->m_id), m_currentSubTemplateType);
	emit selectSubTemplate(m_current->m_id, VICUS::ZoneTemplate::NUM_ST);
}


void SVDBZoneTemplateEditWidget::on_pushButtonAddPersonLoad_clicked() {
	Q_ASSERT(m_current != nullptr);

	// open the Internal Loads DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbInternalLoadsPersonEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID) return;
	if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] != id) {
		if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), VICUS::ZoneTemplate::ST_IntLoadPerson, id);
			emit selectSubTemplate(m_current->m_id, (int)VICUS::ZoneTemplate::ST_IntLoadPerson);
		}
		else {
			// modify existing
			m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] = id;
			m_db->m_zoneTemplates.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}


void SVDBZoneTemplateEditWidget::on_pushButtonAddElectricLoad_clicked() {
	Q_ASSERT(m_current != nullptr);

	// open the Internal Loads DB dialog and let user select one
	unsigned int id = SVMainWindow::instance().dbInternalLoadsElectricEquipmentEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID) return;
	if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] != id) {
		if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), VICUS::ZoneTemplate::ST_IntLoadEquipment, id);
			emit selectSubTemplate(m_current->m_id, (int)VICUS::ZoneTemplate::ST_IntLoadEquipment);
		}
		else {
			// modify existing
			m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadEquipment] = id;
			m_db->m_zoneTemplates.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}
