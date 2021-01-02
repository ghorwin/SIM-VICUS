#include "SVDBBoundaryConditionEditWidget.h"
#include "ui_SVDBBoundaryConditionEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVDBBoundaryConditionTableModel.h"


SVDBBoundaryConditionEditWidget::SVDBBoundaryConditionEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBBoundaryConditionEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	m_ui->lineEditSolarAbsorptionCoefficient->setup(0, 1, tr("Solar Absorption (short wave)"), true, true);
	m_ui->lineEditLongWaveEmissivity->setup(0, 1, tr("Thermal Absorption (long wave)"), true, true);
	m_ui->lineEditHeatTransferCoefficient->setup(0.001, 500, tr("Thermal conductivity"), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBBoundaryConditionEditWidget::~SVDBBoundaryConditionEditWidget() {
	delete m_ui;
}


void SVDBBoundaryConditionEditWidget::setup(SVDatabase * db, SVDBBoundaryConditionTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBBoundaryConditionEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->lineEditSolarAbsorptionCoefficient->setEnabled(isEnabled);
	m_ui->lineEditLongWaveEmissivity->setEnabled(isEnabled);
	m_ui->lineEditHeatTransferCoefficient->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSolarAbsorptionCoefficient->setText("");
		m_ui->lineEditLongWaveEmissivity->setText("");
		m_ui->lineEditHeatTransferCoefficient->setText("");

		return;
	}

	VICUS::BoundaryCondition * bc = const_cast<VICUS::BoundaryCondition *>(m_db->m_boundaryConditions[(unsigned int)id]);
	m_current = bc;

	// now update the GUI controls
	m_ui->lineEditName->setString(bc->m_displayName);

	m_ui->lineEditSolarAbsorptionCoefficient->setValue(bc->m_para[VICUS::BoundaryCondition::P_SolarAbsorption].value);
	m_ui->lineEditLongWaveEmissivity->setValue(bc->m_para[VICUS::BoundaryCondition::P_Emissivity].value);
	m_ui->lineEditHeatTransferCoefficient->setValue(bc->m_para[VICUS::BoundaryCondition::P_HeatTransferCoefficient].value);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (bc->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditSolarAbsorptionCoefficient->setReadOnly(!isEditable);
	m_ui->lineEditLongWaveEmissivity->setReadOnly(!isEditable);
	m_ui->lineEditHeatTransferCoefficient->setReadOnly(!isEditable);
}


void SVDBBoundaryConditionEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_boundaryConditions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditHeatTransferCoefficient_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditHeatTransferCoefficient->isValid() ) {
		double val = m_ui->lineEditHeatTransferCoefficient->value();
		// update database but only if different from original
		if (m_current->m_para[VICUS::BoundaryCondition::P_HeatTransferCoefficient].empty() ||
			val != m_current->m_para[VICUS::BoundaryCondition::P_HeatTransferCoefficient].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "BoundaryCondition::para_t", VICUS::BoundaryCondition::P_HeatTransferCoefficient, val);
			m_db->m_boundaryConditions.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
		}
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditSolarAbsorptionCoefficient_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditSolarAbsorptionCoefficient->isValid() ) {
		double val = m_ui->lineEditSolarAbsorptionCoefficient->value();
		// update database but only if different from original
		if (m_current->m_para[VICUS::BoundaryCondition::P_SolarAbsorption].empty() ||
			val != m_current->m_para[VICUS::BoundaryCondition::P_SolarAbsorption].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "BoundaryCondition::para_t", VICUS::BoundaryCondition::P_SolarAbsorption, val);
			m_db->m_boundaryConditions.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
		}
	}
}


void SVDBBoundaryConditionEditWidget::on_lineEditLongWaveEmissivity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditLongWaveEmissivity->isValid() ) {
		double val = m_ui->lineEditLongWaveEmissivity->value();
		// update database but only if different from original
		if (m_current->m_para[VICUS::BoundaryCondition::P_Emissivity].empty() ||
			val != m_current->m_para[VICUS::BoundaryCondition::P_Emissivity].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "BoundaryCondition::para_t", VICUS::BoundaryCondition::P_Emissivity, val);
			m_db->m_boundaryConditions.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
		}
	}
}


