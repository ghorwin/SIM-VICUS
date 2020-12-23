#include "SVDBComponentEditWidget.h"
#include "ui_SVDBComponentEditWidget.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVSettings.h"
#include "SVDBComponentTableModel.h"

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

SVDBComponentEditWidget::~SVDBComponentEditWidget()
{
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
		m_ui->lineEditName->setEnabled(false);
		m_ui->comboBoxComponentType->setEnabled(false);
		m_ui->pushButtonComponentColor->setEnabled(false);
		m_ui->pushButtonSelectConstruction->setEnabled(false);
		m_ui->pushButtonBoundaryConditionSideAName->setEnabled(false);
		m_ui->pushButtonBoundaryConditionSideBName->setEnabled(false);
		m_ui->pushButtonDaylight->setEnabled(false);

		m_ui->lineEditUValue->setEnabled(false);
		m_ui->lineEditConstructionName->setEnabled(false);

		m_ui->lineEditBoundaryConditionSideAName->setEnabled(false);
		m_ui->lineEditHeatTransferCoeffSideA->setEnabled(false);
		m_ui->lineEditSolarAbsorptionSideA->setEnabled(false);
		m_ui->lineEditThermalAbsorptionSideA->setEnabled(false);

		m_ui->lineEditBoundaryConditionSideBName->setEnabled(false);
		m_ui->lineEditHeatTransferCoeffSideB->setEnabled(false);
		m_ui->lineEditSolarAbsorptionSideB->setEnabled(false);
		m_ui->lineEditThermalAbsorptionSideB->setEnabled(false);

		m_ui->lineEditDaylightName->setEnabled(false);
		m_ui->lineEditRoughness->setEnabled(false);
		m_ui->lineEditSpecularity->setEnabled(false);



		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
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

	VICUS::Component * comp = const_cast<VICUS::Component*>(m_db->m_components[(unsigned int)id]);
	m_current = comp;

	// now update the GUI controls
	// disable all controls

	// clear input controls
	m_ui->lineEditName->setString(comp->m_displayName);

	const VICUS::Construction *con = m_db->m_constructions[comp->m_idOpaqueConstruction];
	if(con != nullptr){
		m_ui->lineEditConstructionName->setText(QString::fromStdString(con->m_displayName.string(IBK::MultiLanguageString::m_language)));
		double UValue;
		bool validUValue = con->calculateUValue(UValue, m_db->m_materials, 0.13, 0.04);
		if(validUValue)
			m_ui->lineEditUValue->setValue(UValue);
		else
			m_ui->lineEditUValue->setText("---");
	}

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setEnabled(true);
	m_ui->comboBoxComponentType->setEnabled(isEditable);
	m_ui->pushButtonSelectConstruction->setEnabled(isEditable);
	m_ui->pushButtonComponentColor->setEnabled(isEditable);
	m_ui->pushButtonBoundaryConditionSideAName->setEnabled(isEditable);
	m_ui->pushButtonBoundaryConditionSideBName->setEnabled(isEditable);
	m_ui->pushButtonDaylight->setEnabled(isEditable);

	///TODO Dirk später durchführen wenn datenbanken da sind

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





void SVDBComponentEditWidget::on_pushButtonSelectConstruction_clicked()
{
	/// TODO Andreas: wie verweise ich jetzt hier auf das construction view dialog fenster und wähle was aus?
}
