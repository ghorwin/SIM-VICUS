#include "SVDBComponentEditWidget.h"
#include "ui_SVDBComponentEditWidget.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVSettings.h"
#include "SVDBComponentTableModel.h"
#include "SVDBConstructionEditDialog.h"
#include "SVDBBoundaryConditionEditDialog.h"
#include "SVMainWindow.h"

SVDBComponentEditWidget::SVDBComponentEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBComponentEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption(tr("Component identification name"));

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxComponentType->blockSignals(true);
	for(int i=0; i<VICUS::Component::NUM_CT; ++i)
		m_ui->comboBoxComponentType->addItem(VICUS::KeywordListQt::Keyword("Component::ComponentType", i), i);
	m_ui->comboBoxComponentType->blockSignals(false);

	//construction group box
	m_ui->lineEditUValue->setReadOnly(true);
	m_ui->lineEditConstructionName->setReadOnly(true);

	//Boundary condition Side A
	m_ui->lineEditHeatTransferCoeffSideA->setReadOnly(true);
	m_ui->lineEditSolarAbsorptionSideA->setReadOnly(true);
	m_ui->lineEditThermalAbsorptionSideA->setReadOnly(true);

	//Boundary condition Side B
	m_ui->lineEditHeatTransferCoeffSideB->setReadOnly(true);
	m_ui->lineEditSolarAbsorptionSideB->setReadOnly(true);
	m_ui->lineEditThermalAbsorptionSideB->setReadOnly(true);

	//Daylight
	m_ui->lineEditDaylightName->setReadOnly(true);
	m_ui->pushButtonDaylightColor->setEnabled(false);
	m_ui->lineEditRoughness->setReadOnly(true);
	m_ui->lineEditSpecularity->setReadOnly(true);


	updateInput(-1);
}


SVDBComponentEditWidget::~SVDBComponentEditWidget() {
	delete m_ui;
}


void SVDBComponentEditWidget::setup(SVDatabase * db, SVDBComponentTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBComponentEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

		// construction property info fields
		m_ui->comboBoxComponentType->setCurrentText("");
		m_ui->lineEditConstructionName->setText("");
		m_ui->lineEditUValue->setText("");

		m_ui->lineEditBoundaryConditionSideAName->setText("");
		m_ui->lineEditHeatTransferCoeffSideA->setText("");
		m_ui->lineEditSolarAbsorptionSideA->setText("");
		m_ui->lineEditThermalAbsorptionSideA->setText("");

		m_ui->lineEditBoundaryConditionSideBName->setText("");
		m_ui->lineEditHeatTransferCoeffSideB->setText("");
		m_ui->lineEditSolarAbsorptionSideB->setText("");
		m_ui->lineEditThermalAbsorptionSideB->setText("");

		m_ui->lineEditDaylightName->setText("");
		m_ui->lineEditRoughness->setText("");
		m_ui->lineEditSpecularity->setText("");

		return;
	}
	// re-enable all controls
	setEnabled(true);

	VICUS::Component * comp = const_cast<VICUS::Component*>(m_db->m_components[(unsigned int)id]);
	m_current = comp;

	// now update the GUI controls
	m_ui->comboBoxComponentType->blockSignals(true);
	m_ui->lineEditName->setString(comp->m_displayName);
	int typeIdx = m_ui->comboBoxComponentType->findData(comp->m_type);
	m_ui->comboBoxComponentType->setCurrentIndex(typeIdx);
	m_ui->comboBoxComponentType->blockSignals(false);

	m_ui->lineEditBoundaryConditionSideAName->setEnabled(true);
	m_ui->lineEditHeatTransferCoeffSideA->setEnabled(true);
	m_ui->lineEditSolarAbsorptionSideA->setEnabled(true);
	m_ui->lineEditThermalAbsorptionSideA->setEnabled(true);
	m_ui->lineEditBoundaryConditionSideAName->setText("");
	m_ui->lineEditHeatTransferCoeffSideA->setText("---");
	m_ui->lineEditSolarAbsorptionSideA->setText("---");
	m_ui->lineEditThermalAbsorptionSideA->setText("---");

	m_ui->lineEditBoundaryConditionSideBName->setEnabled(true);
	m_ui->lineEditHeatTransferCoeffSideB->setEnabled(true);
	m_ui->lineEditSolarAbsorptionSideB->setEnabled(true);
	m_ui->lineEditThermalAbsorptionSideB->setEnabled(true);
	m_ui->lineEditBoundaryConditionSideBName->setText("");
	m_ui->lineEditHeatTransferCoeffSideB->setText("---");
	m_ui->lineEditSolarAbsorptionSideB->setText("---");
	m_ui->lineEditThermalAbsorptionSideB->setText("---");

	const VICUS::Construction *con = m_db->m_constructions[comp->m_idOpaqueConstruction];
	if (con != nullptr) {
		m_ui->lineEditConstructionName->setText(QString::fromStdString(con->m_displayName.string(QtExt::LanguageHandler::langId().toStdString(), "en")));
		double UValue;
		bool validUValue = con->calculateUValue(UValue, m_db->m_materials, 0.13, 0.04);
		if (validUValue)
			m_ui->lineEditUValue->setText(QString("%L1").arg(UValue, 0, 'f', 4));
	}
	else {
		m_ui->lineEditUValue->setText("---");
		m_ui->lineEditConstructionName->setText("");
	}

	const VICUS::BoundaryCondition *bcA = m_db->m_boundaryConditions[comp->m_idOutsideBoundaryCondition];
	if(bcA != nullptr){
		m_ui->lineEditBoundaryConditionSideAName->setText(QString::fromStdString(bcA->m_displayName.string(QtExt::LanguageHandler::langId().toStdString(), std::string("en"))));
		m_ui->lineEditHeatTransferCoeffSideA->setValue(bcA->m_para[VICUS::BoundaryCondition::P_HeatTransferCoefficient].value);
		m_ui->lineEditSolarAbsorptionSideA->setValue(bcA->m_para[VICUS::BoundaryCondition::P_SolarAbsorption].value);
		m_ui->lineEditThermalAbsorptionSideA->setValue(bcA->m_para[VICUS::BoundaryCondition::P_Emissivity].value);
	}

	const VICUS::BoundaryCondition *bcB = m_db->m_boundaryConditions[comp->m_idInsideBoundaryCondition];
	if(bcB != nullptr){
		m_ui->lineEditBoundaryConditionSideBName->setText(QString::fromStdString(bcB->m_displayName.string(QtExt::LanguageHandler::langId().toStdString(), std::string("en"))));
		m_ui->lineEditHeatTransferCoeffSideB->setValue(bcB->m_para[VICUS::BoundaryCondition::P_HeatTransferCoefficient].value);
		m_ui->lineEditSolarAbsorptionSideB->setValue(bcB->m_para[VICUS::BoundaryCondition::P_SolarAbsorption].value);
		m_ui->lineEditThermalAbsorptionSideB->setValue(bcB->m_para[VICUS::BoundaryCondition::P_Emissivity].value);
	}

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setEnabled(true);
	m_ui->comboBoxComponentType->setEnabled(isEditable);
	m_ui->toolButtonSelectConstruction->setEnabled(isEditable);
	m_ui->pushButtonComponentColor->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideAName->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideBName->setEnabled(isEditable);
	m_ui->pushButtonDaylight->setEnabled(isEditable);

	///TODO Dirk später durchführen wenn datenbanken da sind

	m_ui->lineEditDaylightName->setEnabled(true);
	m_ui->lineEditRoughness->setEnabled(true);
	m_ui->lineEditSpecularity->setEnabled(true);
	m_ui->lineEditDaylightName->setText("");
	m_ui->lineEditRoughness->setText("---");
	m_ui->lineEditSpecularity->setText("---");

	//read only?
	m_ui->lineEditName->setReadOnly(!isEditable);

}


void SVDBComponentEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_components.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}


void SVDBComponentEditWidget::on_comboBoxComponentType_currentIndexChanged(int index){
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::Component::ComponentType ct = static_cast<VICUS::Component::ComponentType>(m_ui->comboBoxComponentType->currentData().toInt());
	if (ct != m_current->m_type) {
		m_current->m_type = ct;
		m_db->m_constructions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}


void SVDBComponentEditWidget::on_toolButtonSelectConstruction_clicked() {
	// get construction edit dialog from mainwindow
	SVDBConstructionEditDialog * conEditDialog = SVMainWindow::instance().dbConstructionEditDialog();
	int conId = conEditDialog->select(m_current->m_idOpaqueConstruction);
	if (conId != -1) {
		m_current->m_idOpaqueConstruction = conId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		updateInput(m_current->m_id);
	}
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideAName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDBBoundaryConditionEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	int bcId = bcEditDialog->select(m_current->m_idOutsideBoundaryCondition);
	if (bcId != -1) {
		m_current->m_idOutsideBoundaryCondition= bcId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		updateInput(m_current->m_id);
	}
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideBName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDBBoundaryConditionEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	int bcId = bcEditDialog->select(m_current->m_idInsideBoundaryCondition);
	if (bcId != -1) {
		m_current->m_idInsideBoundaryCondition = bcId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		updateInput(m_current->m_id);
	}
}

