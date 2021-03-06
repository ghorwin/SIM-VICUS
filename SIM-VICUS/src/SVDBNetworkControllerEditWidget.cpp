#include "SVDBNetworkControllerEditWidget.h"
#include "ui_SVDBNetworkControllerEditWidget.h"

#include "SVDBNetworkControllerTableModel.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>

#include <VICUS_KeywordList.h>


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
	m_ui->comboBoxControllerType->clear();
	for (int i=0; i<VICUS::NetworkController::NUM_CT; ++i)
		m_ui->comboBoxControllerType->addItem(QString("%1")
											 .arg(VICUS::KeywordList::Description("NetworkController::ControllerType", i)),
											 i);

	// setup line edits
	m_ui->lineEditKi->setup(0, std::numeric_limits<double>::max(), "Integration Constant", false, true);
	m_ui->lineEditKp->setup(0, std::numeric_limits<double>::max(), "Controller Gain", false, true);
	m_ui->lineEditSetpoint->setup(0, std::numeric_limits<double>::max(), "Set Point", false, true);

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
	const VICUS::Schedule * setPointSched = m_db->m_schedules[m_current->m_scheduleId];

	// controlled property
	int propIdx = m_ui->comboBoxProperty->findData(m_current->m_controlledProperty);
	m_ui->comboBoxProperty->setCurrentIndex(propIdx);

	// set point / schedule
	switch (VICUS::NetworkController::ControlledProperty(propIdx)) {
		case VICUS::NetworkController::CP_TemperatureDifference:
		case VICUS::NetworkController::CP_TemperatureDifferenceOfFollowingElement:{
			m_ui->radioButtonFixedSetPoint->setText(tr("Set Point Temperature Difference [K]"));
			m_ui->radioButtonSchedule->setText(tr("Schedule Temperature Difference [K]"));
			if (m_current->m_modelType == VICUS::NetworkController::MT_Constant)
				m_ui->lineEditSetpoint->setValue(m_current->m_para[VICUS::NetworkController::P_TemperatureDifferenceSetpoint].value);
			else if (setPointSched != nullptr)
				m_ui->lineEditSchedule->setText(QtExt::MultiLangString2QString(setPointSched->m_displayName));
			break;
		}
		case VICUS::NetworkController::CP_MassFlux :{
			m_ui->radioButtonFixedSetPoint->setText(tr("Set Point Mass Flux [kg/s]"));
			m_ui->radioButtonSchedule->setText(tr("Schedule Mass Flux [kg/s]"));
			if (m_current->m_modelType == VICUS::NetworkController::MT_Constant)
				m_ui->lineEditSetpoint->setValue(m_current->m_para[VICUS::NetworkController::P_MassFluxSetpoint].value);
			else if (setPointSched != nullptr)
				m_ui->lineEditSchedule->setText(QtExt::MultiLangString2QString(setPointSched->m_displayName));
			break;
		}
		case VICUS::NetworkController::CP_ThermostatValue:
		case VICUS::NetworkController::NUM_CP:
			break;
	}

	// controller type and parameters
	int typeIdx = m_ui->comboBoxControllerType->findData(m_current->m_controllerType);
	m_ui->comboBoxControllerType->setCurrentIndex(typeIdx);
	m_ui->lineEditKp->setValue(m_current->m_para[VICUS::NetworkController::P_Kp].value);
	m_ui->lineEditKi->setEnabled(m_current->m_controllerType == VICUS::NetworkController::CT_PIController);
	m_ui->lineEditKi->setValue(m_current->m_para[VICUS::NetworkController::P_Ki].value);
	m_ui->radioButtonSchedule->setChecked(m_current->m_modelType == VICUS::NetworkController::MT_Scheduled);
	m_ui->radioButtonFixedSetPoint->setChecked(m_current->m_modelType == VICUS::NetworkController::MT_Constant);
	toggleLineEdits();


	// for built-ins, disable editing/make read-only
	bool isEditable = !m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditSetpoint->setReadOnly(!isEditable);
	m_ui->lineEditSchedule->setReadOnly(!isEditable);
	m_ui->lineEditKp->setReadOnly(!isEditable);
	m_ui->lineEditKi->setReadOnly(!isEditable);
}


void SVDBNetworkControllerEditWidget::modelModify() {
	m_db->m_networkControllers.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id);

}

void SVDBNetworkControllerEditWidget::toggleLineEdits()
{
	m_ui->lineEditSetpoint->setEnabled(m_current->m_modelType == VICUS::NetworkController::MT_Constant);
	m_ui->lineEditSchedule->setEnabled(m_current->m_modelType == VICUS::NetworkController::MT_Scheduled);
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
	switch (m_current->m_controlledProperty) {
		case VICUS::NetworkController::CP_TemperatureDifference:
		case VICUS::NetworkController::CP_TemperatureDifferenceOfFollowingElement:{
			VICUS::KeywordList::setParameter(m_current->m_para, "NetworkController::para_t",
											   VICUS::NetworkController::P_TemperatureDifferenceSetpoint,
											   m_ui->lineEditSetpoint->value());
		} break;

		case VICUS::NetworkController::CP_MassFlux:{
			VICUS::KeywordList::setParameter(m_current->m_para, "NetworkController::para_t",
											   VICUS::NetworkController::P_MassFluxSetpoint,
											   m_ui->lineEditSetpoint->value());
		} break;

		case VICUS::NetworkController::CP_ThermostatValue :
		case VICUS::NetworkController::NUM_CP:
			break;
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
	toggleLineEdits();
}

void SVDBNetworkControllerEditWidget::on_radioButtonFixedSetPoint_clicked(bool checked)
{
	if (checked && (m_current->m_modelType != VICUS::NetworkController::MT_Constant)){
		m_current->m_modelType = VICUS::NetworkController::MT_Constant;
		modelModify();
	}
	toggleLineEdits();
}

void SVDBNetworkControllerEditWidget::on_toolButtonSchedule_clicked()
{
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_scheduleId);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	if (m_current->m_scheduleId != newId) {
		m_current->m_scheduleId = newId;
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
