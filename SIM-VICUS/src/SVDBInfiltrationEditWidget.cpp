#include "SVDBInfiltrationEditWidget.h"
#include "ui_SVDBInfiltrationEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBInfiltrationTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"

SVDBInfiltrationEditWidget::SVDBInfiltrationEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBInfiltrationEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	// *** populate combo boxes ***

	m_ui->comboBoxMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::Infiltration::NUM_AC; ++i) {
		m_ui->comboBoxMethod->addItem(QString("%1 [%2]")
			.arg(VICUS::KeywordListQt::Description("Infiltration::AirChangeType", (int)i))
			.arg(VICUS::KeywordListQt::Keyword("Infiltration::AirChangeType", (int)i)), i);
	}
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Infiltration Model Name"));

	m_ui->lineEditAirChangeRate->setup(0, 100, tr("Houly air change rate of entire zone air volume."), true, true);
	m_ui->lineEditShieldCoefficient->setup(0, 1, tr("Shield coefficient DIN EN 13789."), true, true); //Vorinit auf 0.07?

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBInfiltrationEditWidget::~SVDBInfiltrationEditWidget() {
	delete m_ui;
}


void SVDBInfiltrationEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBInfiltrationTableModel*>(dbModel);
}


void SVDBInfiltrationEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelDisplayName->setText(tr("Name:"));
	m_ui->labelCategory_2->setText(tr("Method:"));
	m_ui->labelAirChangeRate->setText(tr("Air Change Rate:"));
	m_ui->labelShieldCoefficient->setText(tr("Shield Coefficient:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditAirChangeRate->setText("");
		m_ui->lineEditShieldCoefficient->setText("");

		return;
	}

	m_current = const_cast<VICUS::Infiltration *>(m_db->m_infiltration[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);


	//set method
	m_ui->comboBoxMethod->blockSignals(true);
	m_ui->comboBoxMethod->setCurrentIndex(m_current->m_airChangeType);
	m_ui->comboBoxMethod->blockSignals(false);

	try {
		m_ui->lineEditAirChangeRate->setValue(m_current->m_para[VICUS::Infiltration::P_AirChangeRate].get_value(IBK::Unit("1/h")));
	}  catch (IBK::Exception &ex) {
		//set up a new value
		m_ui->lineEditAirChangeRate->setValue(0);
	}
	m_ui->lineEditShieldCoefficient->setValue(m_current->m_para[VICUS::Infiltration::P_ShieldingCoefficient].value);

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditAirChangeRate->setEnabled(!isbuiltIn);
	m_ui->lineEditShieldCoefficient->setEnabled(!isbuiltIn);
}


void SVDBInfiltrationEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBInfiltrationEditWidget::on_comboBoxControlValue_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::Infiltration::AirChangeType::NUM_AC; ++i){
		if(index == i){
			m_current->m_airChangeType = static_cast<VICUS::Infiltration::AirChangeType>(i);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data

		}
	}
}


void SVDBInfiltrationEditWidget::on_lineEditShieldCoefficient_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if(m_ui->lineEditShieldCoefficient->isValid()){
		double val = m_ui->lineEditShieldCoefficient->value();

		VICUS::Infiltration::para_t paraName = VICUS::Infiltration::P_ShieldingCoefficient;
		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Infiltration::para_t", paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}

}

void SVDBInfiltrationEditWidget::on_lineEditAirChangeRate_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if(m_ui->lineEditAirChangeRate->isValid()){
		double val = m_ui->lineEditAirChangeRate->value();

		VICUS::Infiltration::para_t paraName= VICUS::Infiltration::P_AirChangeRate;
		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Infiltration::para_t", paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}


void SVDBInfiltrationEditWidget::modelModify() {
	m_db->m_infiltration.m_modified = true;
}

void SVDBInfiltrationEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}




