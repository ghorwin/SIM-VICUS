#include "SVDBInternalLoadsPersonEditWidget.h"
#include "ui_SVDBInternalLoadsPersonEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>

#include "SVDBInternalLoadTableModel.h"
#include "SVMainWindow.h"
#include "SVDBScheduleEditDialog.h"

SVDBInternalLoadsPersonEditWidget::SVDBInternalLoadsPersonEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBInternalLoadsPersonEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);


	// *** populate combo boxes ***

	m_ui->comboBoxCategory->blockSignals(true);
	//TODO Dirk later
	m_ui->comboBoxCategory->blockSignals(false);

	m_ui->comboBoxPersonMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::InternalLoad::NUM_PCM; ++i) {
		m_ui->comboBoxPersonMethod->addItem(QString("%1 [%2]")
			.arg(VICUS::KeywordListQt::Description("InternalLoad::PersonCountMethod", (int)i))
			.arg(VICUS::KeywordListQt::Keyword("InternalLoad::PersonCountMethod", (int)i)), i);
	}
	m_ui->comboBoxPersonMethod->blockSignals(false);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBInternalLoadsPersonEditWidget::~SVDBInternalLoadsPersonEditWidget() {
	delete m_ui;
}


void SVDBInternalLoadsPersonEditWidget::setup(SVDatabase *db, SVDBInternalLoadTableModel *dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBInternalLoadsPersonEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->labelPersonCountUnit->setText("");
		m_ui->lineEditPersonCount->setText("-");
		m_ui->lineEditConvectiveFactor->setText("-");
		return;
	}

	m_current = const_cast<VICUS::InternalLoad *>(m_db->m_internalLoads[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonPersonColor->setColor(m_current->m_color);
	//change invalid person count method to a valid one
	if (m_current->m_personCountMethod == VICUS::InternalLoad::PersonCountMethod::NUM_PCM){
		m_current->m_personCountMethod = VICUS::InternalLoad::PersonCountMethod::PCM_PersonCount;
		m_db->m_internalLoads.m_modified=true;
	}

	m_ui->comboBoxPersonMethod->blockSignals(true);
	m_ui->comboBoxPersonMethod->setCurrentIndex(m_current->m_personCountMethod);
	m_ui->comboBoxPersonMethod->blockSignals(false);

	m_ui->lineEditPersonCount->setValue(m_current->m_para[VICUS::InternalLoad::P_PersonCount].value);
	m_ui->lineEditConvectiveFactor->setValue(m_current->m_para[VICUS::InternalLoad::P_ConvectiveHeatFactor].value);

	switch (m_current->m_personCountMethod) {
		case VICUS::InternalLoad::PCM_AreaPerPerson:	m_ui->labelPersonCountUnit->setText(tr("m2 per Person"));				break;
		case VICUS::InternalLoad::PCM_PersonPerArea:	m_ui->labelPersonCountUnit->setText(tr("Person per m2"));				break;
		case VICUS::InternalLoad::PCM_PersonCount:
		case VICUS::InternalLoad::NUM_PCM:
			m_ui->labelPersonCountUnit->setText(tr("Person"));						break;
	}

	VICUS::Schedule * occSched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_occupancyScheduleId ]);
	if (occSched != nullptr)
		m_ui->lineEditOccupancyScheduleName->setText(QtExt::MultiLangString2QString(occSched->m_displayName));
	else
		m_ui->lineEditOccupancyScheduleName->setText(tr("<select schedule>"));

	VICUS::Schedule * actSched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_activityScheduleId ]);
	if (actSched != nullptr)
		m_ui->lineEditActivityScheduleName->setText(QtExt::MultiLangString2QString(actSched->m_displayName));
	else
		m_ui->lineEditActivityScheduleName->setText(tr("<select schedule>"));

	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonPersonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxPersonMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditPersonCount->setEnabled(!isbuiltIn);
	m_ui->lineEditConvectiveFactor->setEnabled(!isbuiltIn);
	m_ui->lineEditActivityScheduleName->setEnabled(!isbuiltIn);
	m_ui->lineEditOccupancyScheduleName->setEnabled(!isbuiltIn);
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_internalLoads.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBInternalLoadsPersonEditWidget::on_comboBoxPersonMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::InternalLoad::PersonCountMethod::NUM_PCM; ++i){
		if(index == i){
			m_current->m_personCountMethod = static_cast<VICUS::InternalLoad::PersonCountMethod>(i);
			m_db->m_internalLoads.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data

		}
	}

	m_ui->labelPersonCountUnit->setText(VICUS::KeywordListQt::Description("InternalLoad::PersonCountMethod", index));
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditPersonCount_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditPersonCount->isValid() ) {
		double val = m_ui->lineEditPersonCount->value();
		// update database but only if different from original
		VICUS::InternalLoad::para_t paraName = VICUS::InternalLoad::P_PersonCount;
		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "InternalLoad::para_t", paraName, val);
			m_db->m_internalLoads.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}


void SVDBInternalLoadsPersonEditWidget::on_lineEditConvectiveFactor_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditConvectiveFactor->isValid() ) {
		double val = m_ui->lineEditConvectiveFactor->value();
		// update database but only if different from original
		VICUS::InternalLoad::para_t paraName = VICUS::InternalLoad::P_ConvectiveHeatFactor;
		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "InternalLoad::para_t", paraName, val);
			m_db->m_internalLoads.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}


void SVDBInternalLoadsPersonEditWidget::on_pushButtonPersonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonPersonColor->color()) {
		m_current->m_color = m_ui->pushButtonPersonColor->color();
		m_db->m_internalLoads.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBInternalLoadsPersonEditWidget::on_toolButtonSelectOccupancy_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_occupancyScheduleId);
	if (m_current->m_occupancyScheduleId != newId) {
		m_current->m_occupancyScheduleId = newId;
		m_db->m_internalLoads.m_modified = true;
	}
	updateInput((int)m_current->m_id);
}

void SVDBInternalLoadsPersonEditWidget::on_toolButtonSelectActivity_clicked() {
	// open schedule edit dialog in selection mode
	unsigned int newId = SVMainWindow::instance().dbScheduleEditDialog()->select(m_current->m_activityScheduleId);
	if (m_current->m_activityScheduleId != newId) {
		m_current->m_activityScheduleId = newId;
		m_db->m_internalLoads.m_modified = true;
	}
	updateInput((int)m_current->m_id);
}
