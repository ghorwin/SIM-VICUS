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

SVDBPipeEditWidget::~SVDBPipeEditWidget()
{
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

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditOuterDiameter->setText("");
		m_ui->lineEditWallThickness->setText("");
		m_ui->lineEditWallLambda->setText("");

		return;
	}

	VICUS::NetworkPipe * pipe = const_cast<VICUS::NetworkPipe*>(m_db->m_pipes[(unsigned int)id]);
	m_current = pipe;

	// now update the GUI controls
	m_ui->lineEditName->setString(pipe->m_displayName);

	m_ui->lineEditWallLambda->setValue(pipe->m_lambdaWall);
	m_ui->lineEditOuterDiameter->setValue(pipe->m_diameterOutside);
	m_ui->lineEditWallThickness->setValue(pipe->m_wallThickness);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (pipe->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditWallLambda->setReadOnly(!isEditable);
	m_ui->lineEditOuterDiameter->setReadOnly(!isEditable);
	m_ui->lineEditWallThickness->setReadOnly(!isEditable);
}

void SVDBPipeEditWidget::on_lineEditOuterDiameter_editingFinished()
{

}

void SVDBPipeEditWidget::on_lineEditName_editingFinished()
{
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_pipes.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}
