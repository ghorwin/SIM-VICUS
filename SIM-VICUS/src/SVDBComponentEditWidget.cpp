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

		m_ui->lineEditBoundaryConditionSideBName->setText("");

		m_ui->lineEditDaylightName->setText("");
		m_ui->lineEditRoughness->setText("");
		m_ui->lineEditSpecularity->setText("");
		m_ui->pushButtonComponentColor->setColor(Qt::black);

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

	m_ui->pushButtonComponentColor->blockSignals(true);
	m_ui->pushButtonComponentColor->setColor(m_current->m_color);
	m_ui->pushButtonComponentColor->blockSignals(false);

	m_ui->lineEditBoundaryConditionSideAName->setEnabled(true);
	m_ui->lineEditBoundaryConditionSideBName->setEnabled(true);

	double surfaceResistanceSideA = 0;
	double surfaceResistanceSideB = 0;

	const VICUS::BoundaryCondition *bcA = m_db->m_boundaryConditions[comp->m_idSideABoundaryCondition];
	if (bcA != nullptr){
		m_ui->lineEditBoundaryConditionSideAName->setText(QString::fromStdString(bcA->m_displayName.string(QtExt::LanguageHandler::langId().toStdString(), std::string("en"))));
		// TODO : generate HTML description text with info about boundary condition
		if(bcA->m_heatConduction.m_modelType == NANDRAD::InterfaceHeatConduction::MT_Constant){
			double hc = bcA->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
			surfaceResistanceSideA = hc > 0 ? 1/hc : 0;
		}
	}

	const VICUS::BoundaryCondition *bcB = m_db->m_boundaryConditions[comp->m_idSideBBoundaryCondition];
	if (bcB != nullptr){
		m_ui->lineEditBoundaryConditionSideBName->setText(QString::fromStdString(bcB->m_displayName.string(QtExt::LanguageHandler::langId().toStdString(), std::string("en"))));
		// TODO : generate HTML description text with info about boundary condition
		if(bcB->m_heatConduction.m_modelType == NANDRAD::InterfaceHeatConduction::MT_Constant){
			double hc = bcB->m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value;
			surfaceResistanceSideB = hc > 0 ? 1/hc : 0;
		}
	}

	const VICUS::Construction *con = m_db->m_constructions[comp->m_idConstruction];
	if (con != nullptr) {
		m_ui->lineEditConstructionName->setText(QString::fromStdString(con->m_displayName.string(QtExt::LanguageHandler::langId().toStdString(), "en")));
		double UValue;
		/* Take for uvalue calculation the surface resistance from the side A and B if this exist.
			If all resistance are zero -> take standard resistance of 0.17+0.04 = 0.21
		*/
		bool validUValue = false;
		if(surfaceResistanceSideA>0 || surfaceResistanceSideB>0)
			validUValue = con->calculateUValue(UValue, m_db->m_materials, surfaceResistanceSideA, surfaceResistanceSideB);
		else
			validUValue = con->calculateUValue(UValue, m_db->m_materials, 0.13, 0.04);

		if (validUValue)
			m_ui->lineEditUValue->setText(QString("%L1").arg(UValue, 0, 'f', 4));
	}
	else {
		m_ui->lineEditUValue->setText("---");
		m_ui->lineEditConstructionName->setText("");
	}

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonComponentColor->setReadOnly(!isEditable);
	m_ui->comboBoxComponentType->setEnabled(isEditable);
	m_ui->toolButtonSelectConstruction->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideAName->setEnabled(isEditable);
	m_ui->toolButtonSelectBoundaryConditionSideBName->setEnabled(isEditable);
	m_ui->pushButtonDaylight->setEnabled(isEditable);

	m_ui->lineEditBoundaryConditionSideAName->setReadOnly(!isEditable);
	m_ui->lineEditBoundaryConditionSideBName->setReadOnly(!isEditable);

	///TODO Dirk später durchführen wenn datenbanken da sind

	m_ui->lineEditDaylightName->setEnabled(true);
	m_ui->lineEditRoughness->setEnabled(true);
	m_ui->lineEditSpecularity->setEnabled(true);
	m_ui->lineEditDaylightName->setText("");
	m_ui->lineEditRoughness->setText("---");
	m_ui->lineEditSpecularity->setText("---");


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
	int conId = conEditDialog->select(m_current->m_idConstruction);
	if (conId != -1) {
		m_current->m_idConstruction = conId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		updateInput(m_current->m_id);
	}
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideAName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDBBoundaryConditionEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	int bcId = bcEditDialog->select(m_current->m_idSideABoundaryCondition);
	if (bcId != -1) {
		m_current->m_idSideABoundaryCondition= bcId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		updateInput(m_current->m_id);
	}
}


void SVDBComponentEditWidget::on_toolButtonSelectBoundaryConditionSideBName_clicked() {
	// get boundary condition edit dialog from mainwindow
	SVDBBoundaryConditionEditDialog * bcEditDialog = SVMainWindow::instance().dbBoundaryConditionEditDialog();
	int bcId = bcEditDialog->select(m_current->m_idSideBBoundaryCondition);
	if (bcId != -1) {
		m_current->m_idSideBBoundaryCondition = bcId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		updateInput(m_current->m_id);
	}
}


void SVDBComponentEditWidget::on_pushButtonComponentColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonComponentColor->color()) {
		m_current->m_color = m_ui->pushButtonComponentColor->color();
		m_db->m_components.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}

}

