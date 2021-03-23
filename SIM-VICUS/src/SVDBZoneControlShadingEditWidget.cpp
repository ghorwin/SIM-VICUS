#include "SVDBZoneControlShadingEditWidget.h"
#include "ui_SVDBZoneControlShadingEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBZoneControlShadingTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"

SVDBZoneControlShadingEditWidget::SVDBZoneControlShadingEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBZoneControlShadingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);

	// *** populate combo boxes ***

	m_ui->comboBoxMethod->blockSignals(true);

	for (unsigned int i=0; i<VICUS::ZoneControlShading::NUM_C; ++i) {
		m_ui->comboBoxMethod->addItem(QString("%1 [%2]")
			.arg(VICUS::KeywordListQt::Description("ZoneControlShading::Category", (int)i))
			.arg(VICUS::KeywordListQt::Keyword("ZoneControlShading::Category", (int)i)), i);
	}
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone control Shading model name"));

	m_ui->lineEditEast->setup(0, 1400, tr("Global Radiation Sensor East."), true, true);
	m_ui->lineEditWest->setup(0, 1400, tr("Global Radiation Sensor West."), true, true);
	m_ui->lineEditNorth->setup(0, 1400, tr("Global Radiation Sensor North."), true, true);
	m_ui->lineEditSouth->setup(0, 1400, tr("Global Radiation Sensor South."), true, true);
	m_ui->lineEditDeadBand->setup(0, 1400, tr("Dead Band for all Parameters."), true, true);
	m_ui->lineEditHorizontal->setup(0, 1400, tr("Global Radiation Sensor Horizontal."), true, true);


	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBZoneControlShadingEditWidget::~SVDBZoneControlShadingEditWidget() {
	delete m_ui;
}


void SVDBZoneControlShadingEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBZoneControlShadingTableModel*>(dbModel);
}


void SVDBZoneControlShadingEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelEast->setText(tr("East:"));
	m_ui->labelWest->setText(tr("West:"));
	m_ui->labelNorth->setText(tr("North:"));
	m_ui->labelSouth->setText(tr("South:"));
	m_ui->labelMethod->setText(tr("Method:"));
	m_ui->labelDeadBand->setText(tr("Dead Band:"));
	m_ui->labelDisplayName->setText(tr("Name:"));


	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditEast->setText("");
		m_ui->lineEditWest->setText("");
		m_ui->lineEditNorth->setText("");
		m_ui->lineEditSouth->setText("");
		m_ui->lineEditDeadBand->setText("");
		m_ui->lineEditHorizontal->setText("");
		return;
	}

	m_current = const_cast<VICUS::ZoneControlShading *>(m_db->m_zoneControlShading[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);


	//set method
	m_ui->comboBoxMethod->blockSignals(true);
	m_ui->comboBoxMethod->setCurrentIndex(m_current->m_category);
	m_ui->comboBoxMethod->blockSignals(false);

	m_ui->lineEditEast->setValue(m_current->m_para[VICUS::ZoneControlShading::P_GlobalEast].value);
	m_ui->lineEditWest->setValue(m_current->m_para[VICUS::ZoneControlShading::P_GlobalWest].value);
	m_ui->lineEditSouth->setValue(m_current->m_para[VICUS::ZoneControlShading::P_GlobalSouth].value);
	m_ui->lineEditNorth->setValue(m_current->m_para[VICUS::ZoneControlShading::P_GlobalEast].value);
	m_ui->lineEditHorizontal->setValue(m_current->m_para[VICUS::ZoneControlShading::P_GlobalHorizontal].value);
	m_ui->lineEditDeadBand->setValue(m_current->m_para[VICUS::ZoneControlShading::P_DeadBand].value);


	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->comboBoxMethod->setEnabled(!isbuiltIn);
	m_ui->lineEditEast->setEnabled(!isbuiltIn);
	m_ui->lineEditWest->setEnabled(!isbuiltIn);
	m_ui->lineEditNorth->setEnabled(!isbuiltIn);
	m_ui->lineEditSouth->setEnabled(!isbuiltIn);
	m_ui->lineEditHorizontal->setEnabled(!isbuiltIn);
	m_ui->lineEditDeadBand->setEnabled(!isbuiltIn);

}


void SVDBZoneControlShadingEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBZoneControlShadingEditWidget::on_comboBoxMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	for(int i=0; i<VICUS::ZoneControlShading::Category::NUM_C; ++i){
		if(index == i){
			m_current->m_category = static_cast<VICUS::ZoneControlShading::Category>(i);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data

		}
	}
}


void SVDBZoneControlShadingEditWidget::on_lineEditNorth_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditNorth;
	VICUS::ZoneControlShading::para_t paraName = VICUS::ZoneControlShading::P_GlobalNorth;
	std::string keywordList = "ZoneControlShading::para_t";

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

void SVDBZoneControlShadingEditWidget::on_lineEditSouth_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditSouth;
	VICUS::ZoneControlShading::para_t paraName = VICUS::ZoneControlShading::P_GlobalSouth;
	std::string keywordList = "ZoneControlShading::para_t";

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
void SVDBZoneControlShadingEditWidget::on_lineEditWest_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditWest;
	VICUS::ZoneControlShading::para_t paraName = VICUS::ZoneControlShading::P_GlobalWest;
	std::string keywordList = "ZoneControlShading::para_t";

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

void SVDBZoneControlShadingEditWidget::on_lineEditEast_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditEast;
	VICUS::ZoneControlShading::para_t paraName = VICUS::ZoneControlShading::P_GlobalEast;
	std::string keywordList = "ZoneControlShading::para_t";

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

void SVDBZoneControlShadingEditWidget::on_lineEditHorizontal_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditHorizontal;
	VICUS::ZoneControlShading::para_t paraName = VICUS::ZoneControlShading::P_GlobalHorizontal;
	std::string keywordList = "ZoneControlShading::para_t";

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

void SVDBZoneControlShadingEditWidget::on_lineEditDeadBand_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditDeadBand;
	VICUS::ZoneControlShading::para_t paraName = VICUS::ZoneControlShading::P_DeadBand;
	std::string keywordList = "ZoneControlShading::para_t";

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


void SVDBZoneControlShadingEditWidget::modelModify() {
	m_db->m_zoneControlShading.m_modified = true;
}

void SVDBZoneControlShadingEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}
