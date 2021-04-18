#include "SVDBVentilationNaturalEditWidget.h"
#include "ui_SVDBVentilationNaturalEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBVentilationNaturalTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"

SVDBVentilationNaturalEditWidget::SVDBVentilationNaturalEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBVentilationNaturalEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone natural ventilation model name"));

	m_ui->lineEditAirChangeRate->setup(0, 50, tr("Air change rate."), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBVentilationNaturalEditWidget::~SVDBVentilationNaturalEditWidget() {
	delete m_ui;
}


void SVDBVentilationNaturalEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBVentilationNaturalTableModel*>(dbModel);
}


void SVDBVentilationNaturalEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelAirChangeRate->setText(tr("Air change rate:"));
	m_ui->labelScheduleHeating->setText(tr("Schedule name:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditAirChangeRate->setText("");
		m_ui->lineEditScheduleName->setText("");
		return;
	}

	m_current = const_cast<VICUS::VentilationNatural *>(m_db->m_ventilationNatural[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);

	try {
		m_ui->lineEditAirChangeRate->setValue(m_current->m_para[VICUS::VentilationNatural::P_AirChangeRate].get_value(IBK::Unit("1/h")));
	}  catch (IBK::Exception &ex) {
		m_ui->lineEditAirChangeRate->setValue(0);
	}

	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_scheduleId]);
	if (sched != nullptr)
		m_ui->lineEditScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
	else
		m_ui->lineEditScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->lineEditScheduleName->setEnabled(!isbuiltIn);

	m_ui->lineEditAirChangeRate->setEnabled(!isbuiltIn);
}


void SVDBVentilationNaturalEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBVentilationNaturalEditWidget::on_lineEditAirChangeRate_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditAirChangeRate;
	VICUS::VentilationNatural::para_t paraName = VICUS::VentilationNatural::P_AirChangeRate;
	std::string keywordList = "VentilationNatural::para_t";

	if(lineEdit->isValid()){
		double val = lineEdit->value();

		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}

void SVDBVentilationNaturalEditWidget::modelModify() {
	m_db->m_ventilationNatural.m_modified = true;
}

void SVDBVentilationNaturalEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBVentilationNaturalEditWidget::on_toolButtonSelectSchedule_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_scheduleId);
	if (m_current->m_scheduleId != newId) {
		m_current->m_scheduleId = newId;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}



