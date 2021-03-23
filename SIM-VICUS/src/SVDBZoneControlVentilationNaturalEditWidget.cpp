#include "SVDBZoneControlVentilationNaturalEditWidget.h"
#include "ui_SVDBZoneControlVentilationNaturalEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBZoneControlVentilationNaturalTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"

SVDBZoneControlEditWidget::SVDBZoneControlVentilationNaturalEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBZoneControlVentilationNaturalEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	// *** populate combo boxes ***

	m_ui->comboBoxMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::ZoneControlVentilationNatural::NUM_CV; ++i) {
		m_ui->comboBoxMethod->addItem(QString("%1 [%2]")
			.arg(VICUS::KeywordListQt::Description("ZoneControlVentilationNatural::ControlValue", (int)i))
			.arg(VICUS::KeywordListQt::Keyword("ZoneControlVentilationNatural::ControlValue", (int)i)), i);
	}
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone control VentilationNatural model name"));

	m_ui->lineEditToleranceHeating->setup(0, 50, tr("VentilationNatural tolerance for heating mode."), true, true);
	m_ui->lineEditToleranceCooling->setup(0, 50, tr("VentilationNatural tolerance for cooling mode."), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBZoneControlVentilationNaturalEditWidget::~SVDBZoneControlVentilationNaturalEditWidget() {
	delete m_ui;
}


void SVDBZoneControlVentilationNaturalEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBZoneControlVentilationNaturalTableModel*>(dbModel);
}


void SVDBZoneControlVentilationNaturalEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelCategory_2->setText(tr("Control Type:"));
	m_ui->labelScheduleHeating->setText(tr("Heating Schedule:"));
	m_ui->labelScheduleCooling->setText(tr("Cooling Schedule:"));
	m_ui->labelToleranceHeating->setText(tr("Tolerance:"));
	m_ui->labelToleranceCooling->setText(tr("Tolerance:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditToleranceHeating->setText("");
		m_ui->lineEditToleranceCooling->setText("");
		m_ui->lineEditHeatingScheduleName->setText("");
		m_ui->lineEditCoolingScheduleName->setText("");
		return;
	}

	m_current = const_cast<VICUS::ZoneControlVentilationNatural *>(m_db->m_zoneControlVentilationNatural[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);


	//set method
	m_ui->comboBoxMethod->blockSignals(true);
	m_ui->comboBoxMethod->setCurrentIndex(m_current->m_ctrlVal);
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditToleranceHeating->setValue(m_current->m_para[VICUS::ZoneControlVentilationNatural::P_ToleranceHeating].value);
	m_ui->lineEditToleranceCooling->setValue(m_current->m_para[VICUS::ZoneControlVentilationNatural::P_ToleranceCooling].value);

	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_heatingSetpointScheduleId]);
	if (sched != nullptr)
		m_ui->lineEditHeatingScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	else
		m_ui->lineEditHeatingScheduleName->setText(tr("<select schedule>"));

	VICUS::Schedule * schedC = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_coolingSetpointScheduleId]);
	if (schedC != nullptr)
		m_ui->lineEditCoolingScheduleName->setText(QtExt::MultiLangString2QString(schedC->m_displayName));
	else
		m_ui->lineEditCoolingScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditHeatingScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditCoolingScheduleName->setEnabled(!isbuiltIn);

	m_ui->lineEditToleranceHeating->setEnabled(!isbuiltIn);
	m_ui->lineEditToleranceCooling->setEnabled(!isbuiltIn);
}


void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBZoneControlVentilationNaturalEditWidget::on_comboBoxControlValue_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::ZoneControlVentilationNatural::ControlValue::NUM_CV; ++i){
		if(index == i){
			m_current->m_ctrlVal = static_cast<VICUS::ZoneControlVentilationNatural::ControlValue>(i);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data

		}
	}
}


void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditToleranceHeating_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if(m_ui->lineEditToleranceHeating->isValid()){
		double val = m_ui->lineEditToleranceHeating->value();

		VICUS::ZoneControlVentilationNatural::para_t paraName;
		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "ZoneControlVentilationNatural::para_t", paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}

}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditToleranceCooling_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if(m_ui->lineEditToleranceCooling->isValid()){
		double val = m_ui->lineEditToleranceCooling->value();

		VICUS::ZoneControlVentilationNatural::para_t paraName;
		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "ZoneControlVentilationNatural::para_t", paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::modelModify() {
	m_db->m_zoneControlVentilationNatural.m_modified = true;
}

void SVDBZoneControlVentilationNaturalEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_toolButtonSelectHeatingSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_heatingSetpointScheduleId);
	if (m_current->m_heatingSetpointScheduleId != newId) {
		m_current->m_heatingSetpointScheduleId = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}

void SVDBZoneControlVentilationNaturalEditWidget::on_toolButtonSelectCoolingSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_coolingSetpointScheduleId);
	if (m_current->m_coolingSetpointScheduleId != newId) {
		m_current->m_coolingSetpointScheduleId = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}



