#include "SVDBZoneTemplateEditWidget.h"
#include "ui_SVDBZoneTemplateEditWidget.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>

#include "SVDBZoneTemplateTreeModel.h"
#include "SVConstants.h"
#include "SVSettings.h"

SVDBZoneTemplateEditWidget::SVDBZoneTemplateEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBZoneTemplateEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	// initial state is "nothing selected"
	updateInput(-1, -1);
}


SVDBZoneTemplateEditWidget::~SVDBZoneTemplateEditWidget() {
	delete m_ui;
}


void SVDBZoneTemplateEditWidget::setup(SVDatabase * db, SVDBZoneTemplateTreeModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBZoneTemplateEditWidget::updateInput(int id, int subTemplateId) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);
	m_ui->labelDisplayName->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

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


