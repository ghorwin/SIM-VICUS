#include "SVDBPipeEditWidget.h"
#include "ui_SVDBPipeEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"
#include "SVDBPipeTableModel.h"

#include <VICUS_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>

SVDBPipeEditWidget::SVDBPipeEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBPipeEditWidget)
{
	m_ui->setupUi(this);
}


SVDBPipeEditWidget::~SVDBPipeEditWidget() {
	delete m_ui;
}


void SVDBPipeEditWidget::setup(SVDatabase * db, SVDBPipeTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBPipeEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->lineEditOuterDiameter->setEnabled(isEnabled);
	m_ui->lineEditWallThickness->setEnabled(isEnabled);
	m_ui->lineEditWallLambda->setEnabled(isEnabled);
	m_ui->lineEditWallRoughness->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditOuterDiameter->clear();
		m_ui->lineEditWallThickness->clear();
		m_ui->lineEditWallLambda->clear();
		m_ui->lineEditWallRoughness->clear();

		return;
	}

	VICUS::NetworkPipe * pipe = const_cast<VICUS::NetworkPipe*>(m_db->m_pipes[(unsigned int)id]);
	m_current = pipe;

	// now update the GUI controls
	m_ui->lineEditName->setString(pipe->m_displayName);
	m_ui->lineEditWallLambda->setValue(pipe->m_lambdaWall);
	m_ui->lineEditOuterDiameter->setValue(pipe->m_diameterOutside);
	m_ui->lineEditWallThickness->setValue(pipe->m_wallThickness);
	m_ui->lineEditWallRoughness->setValue(pipe->m_roughness);

	m_ui->pushButtonPipeColor->blockSignals(true);
	m_ui->pushButtonPipeColor->setColor(pipe->m_color);
	m_ui->pushButtonPipeColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (pipe->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditWallLambda->setReadOnly(!isEditable);
	m_ui->lineEditOuterDiameter->setReadOnly(!isEditable);
	m_ui->lineEditWallThickness->setReadOnly(!isEditable);
	m_ui->lineEditWallRoughness->setReadOnly(!isEditable);
	m_ui->pushButtonPipeColor->setReadOnly(!isEditable);
}


void SVDBPipeEditWidget::on_lineEditOuterDiameter_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_ui->lineEditOuterDiameter->isValid()) {
		m_current->m_diameterOutside = m_ui->lineEditOuterDiameter->value();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBPipeEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBPipeEditWidget::on_lineEditWallThickness_editingFinished() {
	if (m_ui->lineEditWallThickness->isValid()) {
		m_current->m_wallThickness = m_ui->lineEditWallThickness->value();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBPipeEditWidget::on_lineEditWallLambda_editingFinished() {
	if (m_ui->lineEditWallLambda->isValid()) {
		m_current->m_lambdaWall = m_ui->lineEditWallLambda->value();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBPipeEditWidget::on_lineEditWallRoughness_editingFinished() {
	if (m_ui->lineEditWallRoughness->isValid()) {
		m_current->m_roughness = m_ui->lineEditWallRoughness->value();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBPipeEditWidget::on_lineEditInsulationThickness_editingFinished(){
	if (m_ui->lineEditInsulationThickness->isValid()) {
		m_current->m_insulationThickness = m_ui->lineEditInsulationThickness->value();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBPipeEditWidget::on_lineEditInsulationLambda_editingFinished(){
	if (m_ui->lineEditInsulationLambda->isValid()) {
		m_current->m_lambdaInsulation = m_ui->lineEditInsulationLambda->value();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBPipeEditWidget::on_pushButtonPipeColor_colorChanged() {

	if (m_current->m_color != m_ui->pushButtonPipeColor->color()) {
		m_current->m_color = m_ui->pushButtonPipeColor->color();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}
