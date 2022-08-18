#include "SVDBNetworkControllerEditWidget.h"
#include "ui_SVDBNetworkControllerEditWidget.h"

#include "SVDBNetworkControllerTableModel.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include <SV_Conversions.h>

#include <VICUS_Schedule.h>
#include <VICUS_KeywordListQt.h>



SVDBNetworkControllerEditWidget::SVDBNetworkControllerEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBNetworkControllerEditWidget)
{
	m_ui->setupUi(this);

	// setup comboboxes
	m_ui->comboBoxProperty->clear();
	for (int i=0; i<VICUS::NetworkController::NUM_CP; ++i)
		m_ui->comboBoxProperty->addItem(QString("%1")
											 .arg(VICUS::KeywordList::Description("NetworkController::ControlledProperty", i)),
											 i);

	// setup line edits
	m_ui->lineEditKi->setup(0, std::numeric_limits<double>::max(), "Integration Constant", false, true);
	m_ui->lineEditKp->setup(0, std::numeric_limits<double>::max(), "Controller Gain", false, true);
	m_ui->lineEditKp->setFormat('e', 0);
	m_ui->lineEditSetpoint->setup(0, std::numeric_limits<double>::max(), "Set Point", false, true);
	m_ui->lineEditMaxControllerResultValue->setup(0, std::numeric_limits<double>::max(), "Max Y", true, true);
	m_ui->lineEditMaxControllerResultValue->setFormat('e', 0);

}

SVDBNetworkControllerEditWidget::~SVDBNetworkControllerEditWidget()
{
	delete m_ui;
}


void SVDBNetworkControllerEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBNetworkControllerTableModel*>(dbModel);
}


void SVDBNetworkControllerEditWidget::updateInput(int id) {

	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls - note: this does not disable signals of the components below
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSetpoint->clear();
		m_ui->lineEditSchedule->clear();
		m_ui->lineEditSchedule->setReadOnly(true);
		m_ui->lineEditKp->clear();
		m_ui->lineEditKi->clear();
		m_ui->comboBoxProperty->setCurrentIndex(-1);
		m_ui->comboBoxControllerType->setCurrentIndex(-1);

		// Note: color button is disabled, hence color is gray
		return;
	}

	// re-enable all controls
	setEnabled(true);

	// get current controller
	VICUS::NetworkController * ctrl = const_cast<VICUS::NetworkController*>(m_db->m_networkControllers[(unsigned int)id]);
	m_current = ctrl;

	// now update the GUI controls
	m_ui->lineEditName->setString(m_current->m_displayName);

	// get schedule
	const VICUS::Schedule * setPointSched = m_db->m_schedules[m_current->m_idReferences[VICUS::NetworkController::ID_Schedule]];

	// controlled property
	int propIdx = m_ui->comboBoxProperty->findData(m_current->m_controlledProperty);
	m_ui->comboBoxProperty->setCurrentIndex(propIdx);
	VICUS::NetworkController::ControlledProperty controlledProperty = VICUS::NetworkController::ControlledProperty(propIdx);

	// define some properties based on current controlled property
	QString controlledPropertyName;
	bool schedulePossible = false;
	bool maxControllerValuePossible = false;
	switch (controlledProperty) {
		case VICUS::NetworkController::CP_TemperatureDifference:
		case VICUS::NetworkController::CP_TemperatureDifferenceOfFollowingElement:{
			controlledPropertyName = tr("Temperature Difference [K]");
			schedulePossible = true;
			maxControllerValuePossible = true;
		} break;
		case VICUS::NetworkController::CP_MassFlux :{
			controlledPropertyName = tr("Mass Flux [kg/s]");
			schedulePossible = true;
			maxControllerValuePossible = true;
		} break;
		case VICUS::NetworkController::CP_PumpOperation :{
			controlledPropertyName = tr("Heat Flux Threshold of Following Element [W]");
			schedulePossible = false;
			maxControllerValuePossible = false;
		} break;
		case VICUS::NetworkController::CP_PressureDifferenceWorstpoint :{
			controlledPropertyName = tr("Pressure Difference at worst point [Pa]");
			schedulePossible = false;
			maxControllerValuePossible = false;
		} break;
		case VICUS::NetworkController::CP_ThermostatValue: {
			controlledPropertyName = tr("Thermostat Value [-]");
			schedulePossible = false;
			maxControllerValuePossible = false;
		} break;
		case VICUS::NetworkController::NUM_CP:
			break;
	}

	// enable states and text
	m_ui->radioButtonFixedSetPoint->setText(tr("Set point ") + controlledPropertyName);
	m_ui->radioButtonSchedule->setEnabled(schedulePossible);
	if (schedulePossible)
		m_ui->radioButtonSchedule->setText(tr("Schedule ") + controlledPropertyName);
	else
		m_ui->radioButtonSchedule->setText("");
	m_ui->toolButtonSchedule->setEnabled(m_ui->radioButtonSchedule->isChecked());
	m_ui->groupBoxMaximumOutput->setEnabled(maxControllerValuePossible);
	m_ui->toolButtonRemoveSchedule->setEnabled(setPointSched != nullptr);

	// update content
	m_ui->lineEditSetpoint->clear();
	m_ui->lineEditSchedule->clear();
	if (m_current->m_modelType == VICUS::NetworkController::MT_Constant)
		m_ui->lineEditSetpoint->setValue(m_current->m_para[VICUS::NetworkController::setPointType(controlledProperty)].value);
	else if (setPointSched != nullptr)
		m_ui->lineEditSchedule->setText(QtExt::MultiLangString2QString(setPointSched->m_displayName));

	// setup combobox controller type
	m_ui->comboBoxControllerType->clear();
	std::vector<NANDRAD::HydraulicNetworkControlElement::ControllerType> availableCtrTypes =
			NANDRAD::HydraulicNetworkControlElement::availableControllerTypes(
				NANDRAD::HydraulicNetworkControlElement::ControlledProperty(m_current->m_controlledProperty));
	for (int i: availableCtrTypes)
		m_ui->comboBoxControllerType->addItem(QString("%1").arg(VICUS::KeywordList::Description("NetworkController::ControllerType", i)), i);

	// controller type and parameters
	int typeIdx = m_ui->comboBoxControllerType->findData(m_current->m_controllerType);
	m_ui->comboBoxControllerType->setCurrentIndex(typeIdx);
	m_ui->lineEditKp->setEnabled(m_current->m_controllerType == VICUS::NetworkController::CT_PController ||
								 m_current->m_controllerType == VICUS::NetworkController::CT_PIController );
	m_ui->lineEditKp->setValue(m_current->m_para[VICUS::NetworkController::P_Kp].value);
	m_ui->lineEditKi->setEnabled(m_current->m_controllerType == VICUS::NetworkController::CT_PIController);
	m_ui->lineEditKi->setValue(m_current->m_para[VICUS::NetworkController::P_Ki].value);
	m_ui->lineEditMaxControllerResultValue->setValue(m_current->m_maximumControllerResultValue);
	m_ui->radioButtonSchedule->setChecked(m_current->m_modelType == VICUS::NetworkController::MT_Scheduled && !schedulePossible);
	m_ui->radioButtonFixedSetPoint->setChecked(m_current->m_modelType == VICUS::NetworkController::MT_Constant);
	m_ui->lineEditSetpoint->setEnabled(m_current->m_modelType == VICUS::NetworkController::MT_Constant);

	// for built-ins, disable editing/make read-only
	bool isEditable = !m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditSetpoint->setReadOnly(!isEditable);
	m_ui->lineEditKp->setReadOnly(!isEditable);
	m_ui->lineEditKi->setReadOnly(!isEditable);
}


void SVDBNetworkControllerEditWidget::modelModify() {
	m_db->m_networkControllers.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id);

}


void SVDBNetworkControllerEditWidget::on_lineEditName_editingFinished()
{
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBNetworkControllerEditWidget::on_lineEditSetpoint_editingFinished()
{
	if (!m_ui->lineEditSetpoint->isValid())
		return;

	Q_ASSERT(m_current != nullptr);
	VICUS::NetworkController::para_t setPointParaType = VICUS::NetworkController::setPointType(m_current->m_controlledProperty);
	if (setPointParaType != VICUS::NetworkController::NUM_P) {
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkController::para_t",
										   setPointParaType,
										   m_ui->lineEditSetpoint->value());
	}
	modelModify();
}

void SVDBNetworkControllerEditWidget::on_lineEditKp_editingFinished()
{
	if (!m_ui->lineEditKp->isValid())
		return;
	Q_ASSERT(m_current != nullptr);
	double val = m_ui->lineEditKp->value();
	if (m_current->m_para[VICUS::NetworkController::P_Kp].value < val ||
		m_current->m_para[VICUS::NetworkController::P_Kp].value > val ){
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkController::para_t",
										   VICUS::NetworkController::P_Kp, val);
		modelModify();
	}
}

void SVDBNetworkControllerEditWidget::on_lineEditKi_editingFinished()
{
	if (!m_ui->lineEditKi->isValid())
		return;
	Q_ASSERT(m_current != nullptr);
	double val = m_ui->lineEditKi->value();
	if (m_current->m_para[VICUS::NetworkController::P_Ki].value < val ||
		m_current->m_para[VICUS::NetworkController::P_Ki].value > val ){
		VICUS::KeywordList::setParameter(m_current->m_para, "NetworkController::para_t",
										   VICUS::NetworkController::P_Ki, val);
		modelModify();
	}
}


void SVDBNetworkControllerEditWidget::on_radioButtonSchedule_clicked(bool checked)
{
	if (checked && (m_current->m_modelType != VICUS::NetworkController::MT_Scheduled)){
		m_current->m_modelType = VICUS::NetworkController::MT_Scheduled;
		modelModify();
	}
	m_ui->lineEditSetpoint->setEnabled(false);
	m_ui->toolButtonSchedule->setEnabled(checked);
}

void SVDBNetworkControllerEditWidget::on_radioButtonFixedSetPoint_clicked(bool checked)
{
	if (checked && (m_current->m_modelType != VICUS::NetworkController::MT_Constant)){
		m_current->m_modelType = VICUS::NetworkController::MT_Constant;
		modelModify();
	}
	m_ui->lineEditSetpoint->setEnabled(true);
	m_ui->toolButtonSchedule->setEnabled(false);
}

void SVDBNetworkControllerEditWidget::on_toolButtonSchedule_clicked()
{
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_idReferences[VICUS::NetworkController::ID_Schedule]);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	if (m_current->m_idReferences[VICUS::NetworkController::ID_Schedule] != newId) {
		m_current->m_idReferences[VICUS::NetworkController::ID_Schedule] = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBNetworkControllerEditWidget::on_comboBoxProperty_activated(int /*index*/)
{
	Q_ASSERT(m_current != nullptr);
	unsigned int val = m_ui->comboBoxProperty->currentData().toUInt();
	if (m_current->m_controlledProperty != val) {
		m_current->m_controlledProperty = VICUS::NetworkController::ControlledProperty(val);
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBNetworkControllerEditWidget::on_comboBoxControllerType_activated(int /*index*/)
{
	Q_ASSERT(m_current != nullptr);
	unsigned int val = m_ui->comboBoxControllerType->currentData().toUInt();
	if (m_current->m_controllerType != val) {
		m_current->m_controllerType = VICUS::NetworkController::ControllerType(val);
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBNetworkControllerEditWidget::on_pushButtonColor_clicked()
{
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}

void SVDBNetworkControllerEditWidget::on_lineEditMaxControllerResultValue_editingFinished()
{
	if (!m_ui->lineEditMaxControllerResultValue->isValid())
		return;
	Q_ASSERT(m_current != nullptr);
	double val = m_ui->lineEditMaxControllerResultValue->value();
	if (m_current->m_maximumControllerResultValue < val ||
		m_current->m_maximumControllerResultValue > val ){
		m_current->m_maximumControllerResultValue = val;
		modelModify();
	}
}

void SVDBNetworkControllerEditWidget::on_toolButtonRemoveSchedule_clicked()
{
	Q_ASSERT(m_current != nullptr);
	m_current->m_idReferences[VICUS::NetworkController::ID_Schedule] = VICUS::INVALID_ID;
	modelModify();
	updateInput((int)m_current->m_id);
}

