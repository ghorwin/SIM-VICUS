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

	m_ui->lineEditSolarAbsorption->setup(0, 1, tr("Solar Absorption (short wave)"), true, true);
	m_ui->lineEditThermalAbsorption->setup(0, 1, tr("Thermal Absorption (long wave)"), true, true);
	m_ui->lineEditHeatTransferCoefficient->setup(0.001, 500, tr("Thermal conductivity"), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}

SVDBBoundaryConditionEditWidget::~SVDBBoundaryConditionEditWidget()
{
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
	m_ui->lineEditSolarAbsorption->setEnabled(isEnabled);
	m_ui->lineEditThermalAbsorption->setEnabled(isEnabled);
	m_ui->lineEditHeatTransferCoefficient->setEnabled(isEnabled);

	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSolarAbsorption->setText("");
		m_ui->lineEditThermalAbsorption->setText("");
		m_ui->lineEditHeatTransferCoefficient->setText("");

		return;
	}

	VICUS::BoundaryCondition * bc = const_cast<VICUS::BoundaryCondition *>(m_db->m_boundaryConditions[(unsigned int)id]);
	m_current = bc;

	// now update the GUI controls
	m_ui->lineEditName->setString(bc->m_displayName);

	m_ui->lineEditSolarAbsorption->setValue(bc->m_para[VICUS::BoundaryCondition::P_SolarAbsorption].value);
	m_ui->lineEditThermalAbsorption->setValue(bc->m_para[VICUS::BoundaryCondition::P_Emissivity].value);
	m_ui->lineEditHeatTransferCoefficient->setValue(bc->m_para[VICUS::BoundaryCondition::P_HeatTransferCoefficient].value);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (bc->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditSolarAbsorption->setReadOnly(!isEditable);
	m_ui->lineEditThermalAbsorption->setReadOnly(!isEditable);
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

//void SVDBBoundaryConditionEditWidget::on_lineEditDataSource_editingFinished() {
//	Q_ASSERT(m_current != nullptr);

//	if (m_current->m_dataSource != m_ui->lineEditDataSource->string()) {
//		m_current->m_dataSource = m_ui->lineEditDataSource->string();
//		m_db->m_boundaryCondition.m_modified = true;
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//		emit tableDataChanged();
//	}
//}

//void SVDBBoundaryConditionEditWidget::on_lineEditManufacturer_editingFinished() {
//	Q_ASSERT(m_current != nullptr);

//	if (m_current->m_manufacturer != m_ui->lineEditManufacturer->string()) {
//		m_current->m_manufacturer = m_ui->lineEditManufacturer->string();
//		m_db->m_boundaryCondition.m_modified = true;
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//		emit tableDataChanged();
//	}
//}

//void SVDBBoundaryConditionEditWidget::on_lineEditNotes_editingFinished() {
//	Q_ASSERT(m_current != nullptr);

//	if (m_current->m_notes != m_ui->lineEditNotes->string()) {
//		m_current->m_notes = m_ui->lineEditNotes->string();
//		m_db->m_boundaryCondition.m_modified = true;
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//		emit tableDataChanged();
//	}
//}

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

void SVDBBoundaryConditionEditWidget::on_lineEditSolarAbsorption_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditSolarAbsorption->isValid() ) {
		double val = m_ui->lineEditSolarAbsorption->value();
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

void SVDBBoundaryConditionEditWidget::on_lineEditThermalAbsorption_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditThermalAbsorption->isValid() ) {
		double val = m_ui->lineEditThermalAbsorption->value();
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


