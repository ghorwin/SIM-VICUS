/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVDBMaterialEditWidget.h"
#include "ui_SVDBMaterialEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"
#include "SVConstants.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVStyle.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include "SVDBMaterialTableModel.h"


void setTableWidgetItem(QTableWidget *table, std::map<VICUS::EpdCategoryDataset::Module, QTableWidgetItem*> &moduleToTableItem,
						VICUS::EpdCategoryDataset::Module module, VICUS::LcaSettings::CertificationType ct, VICUS::LcaSettings::Module mod, unsigned int row) {
	QTableWidgetItem *item = new QTableWidgetItem;
	item->setText(VICUS::KeywordList::Description("EpdCategoryDataset::Module", module));

	bool isNotUsed = !(ct & mod);
	item->setData(Qt::UserRole, !isNotUsed);
	item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable);
	table->setItem(row, 0, item);

	if(isNotUsed)
		item->setTextColor(Qt::darkGray);

	moduleToTableItem[module] = item;
}


SVDBMaterialEditWidget::SVDBMaterialEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBMaterialEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(4);
	// adjust color botton width
	m_ui->pushButtonColor->setMaximumSize(m_ui->lineEditName->height(), m_ui->lineEditName->height());
	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Material identification name"));


	m_ui->lineEditDataSource->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(),THIRD_LANGUAGE, true);
	m_ui->lineEditDataSource->setDialog3Caption(tr("Data source information"));
	m_ui->lineEditManufacturer->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditManufacturer->setDialog3Caption(tr("Manufacturer name"));
	m_ui->lineEditNotes->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditNotes->setDialog3Caption(tr("Notes"));

	m_ui->lineEditDensity->setup(1, 10000, tr("Density"), true, true);
	m_ui->lineEditConductivity->setup(0.001, 500, tr("Thermal conductivity"), true, true);
	m_ui->lineEditSpecHeatCapacity->setup(200, 5000, tr("Specific heat capacity"), true, true);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetCatA);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetCatB);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetCatC);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetCatD);

	m_ui->tableWidgetCatA->setStyleSheet("QTableWidget::item { padding: 0px }");
	m_ui->tableWidgetCatB->setStyleSheet("QTableWidget::item { padding: 0px }");
	m_ui->tableWidgetCatC->setStyleSheet("QTableWidget::item { padding: 0px }");
	m_ui->tableWidgetCatD->setStyleSheet("QTableWidget::item { padding: 0px }");

	m_ui->tableWidgetCatA->horizontalHeader()->hide();
	m_ui->tableWidgetCatB->horizontalHeader()->hide();
	m_ui->tableWidgetCatC->horizontalHeader()->hide();
	m_ui->tableWidgetCatD->horizontalHeader()->hide();

	m_ui->tableWidgetCatA->setColumnCount(1);
	m_ui->tableWidgetCatB->setColumnCount(1);
	m_ui->tableWidgetCatC->setColumnCount(1);
	m_ui->tableWidgetCatD->setColumnCount(1);

	m_ui->tableWidgetCatA->setRowCount(NUM_CA);
	m_ui->tableWidgetCatB->setRowCount(NUM_CB);
	m_ui->tableWidgetCatC->setRowCount(NUM_CC);
	m_ui->tableWidgetCatD->setRowCount(NUM_CD);


	m_lcaSettings = &SVProjectHandler::instance().project().m_lcaSettings;

	m_ui->tableWidgetCatA->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetCatB->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetCatC->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetCatD->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxCategory->blockSignals(true);
	for (int i=0; i<VICUS::Material::NUM_MC; ++i)
		m_ui->comboBoxCategory->addItem(VICUS::KeywordListQt::Keyword("Material::Category", i), i);
	m_ui->comboBoxCategory->blockSignals(false);

	// color button size fix (overriding style settings)

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBMaterialEditWidget::~SVDBMaterialEditWidget() {
	delete m_ui;
}


void SVDBMaterialEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBMaterialTableModel*>(dbModel);
}


void SVDBMaterialEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls
		m_ui->tabWidgetThermal->setEnabled(false); // makes all inactive

		// clear input controls
		m_ui->lineEditDensity->setText("");
		m_ui->lineEditConductivity->setText("");
		m_ui->lineEditSpecHeatCapacity->setText("");
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditDataSource->setString(IBK::MultiLanguageString());
		m_ui->lineEditManufacturer->setString(IBK::MultiLanguageString());
		m_ui->lineEditNotes->setString(IBK::MultiLanguageString());

		return;
	}
	m_ui->tabWidgetThermal->setEnabled(true);

	VICUS::Material * mat = const_cast<VICUS::Material *>(m_db->m_materials[(unsigned int)id]);
	m_current = mat;

	setTableWidgetItem(m_ui->tableWidgetCatA, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_A1, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_A1, CA_A1);
	setTableWidgetItem(m_ui->tableWidgetCatA, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_A2, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_A2, CA_A2);
	setTableWidgetItem(m_ui->tableWidgetCatA, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_A3, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_A3, CA_A3);
	setTableWidgetItem(m_ui->tableWidgetCatA, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_A4, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_A4, CA_A4);
	setTableWidgetItem(m_ui->tableWidgetCatA, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_A5, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_A5, CA_A5);

	setTableWidgetItem(m_ui->tableWidgetCatB, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_B1, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_B1, CB_B1);
	setTableWidgetItem(m_ui->tableWidgetCatB, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_B2, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_B2, CB_B2);
	setTableWidgetItem(m_ui->tableWidgetCatB, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_B3, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_B3, CB_B3);
	setTableWidgetItem(m_ui->tableWidgetCatB, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_B4, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_B4, CB_B4);
	setTableWidgetItem(m_ui->tableWidgetCatB, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_B5, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_B5, CB_B5);
	setTableWidgetItem(m_ui->tableWidgetCatB, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_B6, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_B6, CB_B6);
	setTableWidgetItem(m_ui->tableWidgetCatB, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_B7, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_B7, CB_B7);

	setTableWidgetItem(m_ui->tableWidgetCatC, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_C1, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_C1, CC_C1);
	setTableWidgetItem(m_ui->tableWidgetCatC, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_C2, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_C2, CC_C2);
	setTableWidgetItem(m_ui->tableWidgetCatC, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_C3, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_C3, CC_C3);
	setTableWidgetItem(m_ui->tableWidgetCatC, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_C4, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_C4, CC_C4);

	setTableWidgetItem(m_ui->tableWidgetCatD, m_moduleToTableItem, VICUS::EpdCategoryDataset::M_D, m_lcaSettings->m_certificationModules, VICUS::LcaSettings::M_D, CD_D);

	// now update the GUI controls
	m_ui->lineEditDensity->setValue(mat->m_para[VICUS::Material::P_Density].value);
	m_ui->lineEditConductivity->setValue(mat->m_para[VICUS::Material::P_Conductivity].value);
	m_ui->lineEditSpecHeatCapacity->setValue(mat->m_para[VICUS::Material::P_HeatCapacity].value);
	m_ui->lineEditName->setString(mat->m_displayName);
	m_ui->lineEditDataSource->setString(mat->m_dataSource);
	m_ui->lineEditManufacturer->setString(mat->m_manufacturer);
	m_ui->lineEditNotes->setString(mat->m_notes);
	m_ui->comboBoxCategory->setCurrentIndex(mat->m_category);
	m_ui->pushButtonColor->setColor(mat->m_color);

	unsigned int idEpdCatA = mat->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA];
	unsigned int idEpdCatB = mat->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryB];
	unsigned int idEpdCatC = mat->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryC];
	unsigned int idEpdCatD = mat->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD];

	m_ui->lineEditCatA->setText("");
	m_ui->lineEditCatB->setText("");
	m_ui->lineEditCatC->setText("");
	m_ui->lineEditCatD->setText("");

	for(std::pair<VICUS::EpdCategoryDataset::Module, QTableWidgetItem*> tableWidgetItem : m_moduleToTableItem) {
		if(tableWidgetItem.second != nullptr) {
			if(tableWidgetItem.second->data(Qt::UserRole) == true) {
				tableWidgetItem.second->setData(Qt::UserRole+1, false);
				tableWidgetItem.second->setBackgroundColor(Qt::darkRed);
			}
			else
				tableWidgetItem.second->setBackground(QBrush(""));
		}
	}

	updateCategory(idEpdCatA, *mat, VICUS::LcaSettings::L_CategoryA, m_ui->lineEditCatA);
	updateCategory(idEpdCatB, *mat, VICUS::LcaSettings::L_CategoryB, m_ui->lineEditCatB);
	updateCategory(idEpdCatC, *mat, VICUS::LcaSettings::L_CategoryC, m_ui->lineEditCatC);
	updateCategory(idEpdCatD, *mat, VICUS::LcaSettings::L_CategoryD, m_ui->lineEditCatD);

	updateCategoryValidity(idEpdCatA, VICUS::LcaSettings::L_CategoryA);
	updateCategoryValidity(idEpdCatB, VICUS::LcaSettings::L_CategoryB);
	updateCategoryValidity(idEpdCatC, VICUS::LcaSettings::L_CategoryC);
	updateCategoryValidity(idEpdCatD, VICUS::LcaSettings::L_CategoryD);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (mat->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->lineEditDataSource->setReadOnly(!isEditable);
	m_ui->lineEditManufacturer->setReadOnly(!isEditable);
	m_ui->lineEditNotes->setReadOnly(!isEditable);
	m_ui->lineEditDensity->setReadOnly(!isEditable);
	m_ui->lineEditConductivity->setReadOnly(!isEditable);
	m_ui->lineEditSpecHeatCapacity->setReadOnly(!isEditable);
	m_ui->comboBoxCategory->setEnabled(isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
}


void SVDBMaterialEditWidget::setCurrentTabIndex(int idx) {
	m_ui->tabWidgetThermal->setCurrentIndex(idx);
}


void SVDBMaterialEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_lineEditDataSource_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_dataSource != m_ui->lineEditDataSource->string()) {
		m_current->m_dataSource = m_ui->lineEditDataSource->string();
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_lineEditManufacturer_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_manufacturer != m_ui->lineEditManufacturer->string()) {
		m_current->m_manufacturer = m_ui->lineEditManufacturer->string();
		m_db->m_materials.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBMaterialEditWidget::on_lineEditNotes_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_notes != m_ui->lineEditNotes->string()) {
		m_current->m_notes = m_ui->lineEditNotes->string();
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_lineEditConductivity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditConductivity->isValid() ) {
		double val = m_ui->lineEditConductivity->value();
		// update database but only if different from original
		if (m_current->m_para[VICUS::Material::P_Conductivity].empty() ||
				val != m_current->m_para[VICUS::Material::P_Conductivity].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Material::para_t", VICUS::Material::P_Conductivity, val);
			modelModify();
		}
	}
}


void SVDBMaterialEditWidget::on_lineEditDensity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditDensity->isValid() ) {
		double val = m_ui->lineEditDensity->value();
		// update database but only if different from original
		VICUS::Material::para_t paraName = VICUS::Material::P_Density;
		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Material::para_t", paraName, val);
			modelModify();
		}
	}

}


void SVDBMaterialEditWidget::on_lineEditSpecHeatCapacity_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if ( m_ui->lineEditSpecHeatCapacity->isValid() ) {
		double val = m_ui->lineEditSpecHeatCapacity->value();
		VICUS::Material::para_t paraName = VICUS::Material::P_HeatCapacity;
		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, "Material::para_t", paraName, val);
			modelModify();
		}
	}
}


void SVDBMaterialEditWidget::on_comboBoxCategory_currentIndexChanged(int index){
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_category)
	{
		m_current->m_category = static_cast<VICUS::Material::Category>(index);
		modelModify();
	}
}


void SVDBMaterialEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}

void SVDBMaterialEditWidget::modelModify() {
	m_db->m_materials.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}

void SVDBMaterialEditWidget::updateCategory(unsigned int idCategory, VICUS::Material &mat,
											const VICUS::LcaSettings::LcaCategory category, QLineEdit *lineEdit) {
	if(mat.m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA] != VICUS::INVALID_ID) {
		VICUS::EpdDataset * epd = const_cast<VICUS::EpdDataset *>(m_db->m_epdDatasets[idCategory]);
		if(epd != nullptr) {
			lineEdit->setText(QtExt::MultiLangString2QString(epd->m_displayName));
			// Update combobox
			for(unsigned int i=0; i<epd->m_epdCategoryDataset.size(); ++i) {
				std::vector<VICUS::EpdCategoryDataset::Module> &modules = epd->m_epdCategoryDataset[i].m_modules;
				for(unsigned int j=0; j<modules.size(); ++j) {
					if(m_moduleToTableItem[modules[j]] != nullptr) {
						QString keyword = VICUS::KeywordList::Keyword("EpdCategoryDataset::Module", modules[j]);
						if(!keyword.startsWith(VICUS::KeywordList::Keyword("LcaSettings::LcaCategory", category)))
							continue;

						if(m_moduleToTableItem[modules[j]]->data(Qt::UserRole) == true) {
							m_moduleToTableItem[modules[j]]->setData(Qt::UserRole+1, true);
							m_moduleToTableItem[modules[j]]->setBackgroundColor(Qt::darkGreen);
						}
					}
				}
			}
		}
	}
}


void SVDBMaterialEditWidget::updateCategoryValidity(unsigned int idCategory, const VICUS::LcaSettings::LcaCategory category) {
	bool isValid = true;

	for(std::pair<VICUS::EpdCategoryDataset::Module, QTableWidgetItem*> tableWidgetItem : m_moduleToTableItem) {
		if(tableWidgetItem.second != nullptr) {
			if(!m_lcaSettings->isLcaCategoryDefined(tableWidgetItem.first))
				continue;

			QString key = VICUS::KeywordList::Keyword("EpdCategoryDataset::Module", tableWidgetItem.first);
			if(!key.startsWith(VICUS::KeywordList::Keyword("LcaSettings::LcaCategory", category)))
				continue;

			if(tableWidgetItem.second->data(Qt::UserRole).toBool() && !tableWidgetItem.second->data(Qt::UserRole+1).toBool()) {
				isValid = false;
				break;
			}
		}
	}

	if(isValid)
		m_ui->tabWidgetLca->setTabIcon(category, QIcon(":/gfx/actions/16x16/ok.png"));
	else
		m_ui->tabWidgetLca->setTabIcon(category, QIcon(":/gfx/actions/16x16/error.png"));

}


void SVDBMaterialEditWidget::on_toolButtonSelectCatA_clicked() {
	// get EPD edit dialog from mainwindow
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();
	unsigned int idCatA = editDialog->select(m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA], false, "A", 4);
	if (idCatA != VICUS::INVALID_ID && idCatA != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA]) {
		VICUS::EpdDataset * epd = const_cast<VICUS::EpdDataset *>(m_db->m_epdDatasets[idCatA]);

		//		if(epd->m_module != VICUS::EpdDataset::M_A1 &&
		//				epd->m_module != VICUS::EpdDataset::M_A1_A2  &&
		//				epd->m_module != VICUS::EpdDataset::M_A1_A3  &&
		//				epd->m_module != VICUS::EpdDataset::M_A2  &&
		//				epd->m_module != VICUS::EpdDataset::M_A3  &&
		//				epd->m_module != VICUS::EpdDataset::M_A4  &&
		//				epd->m_module != VICUS::EpdDataset::M_A5 )
		//		{
		//			std::string keyword = VICUS::KeywordList::Keyword("EPDDataset::Module", epd->m_module);
		//			if (QMessageBox::warning(this, tr("Material EPD Assignement"),
		//										   tr("You want to assign an EPD with Module '%1' to Category A.\n"
		//											  "Are you sure to do this?").arg(QString::fromStdString(keyword)),
		//										   QMessageBox::Yes | QMessageBox::Discard) == QMessageBox::Discard)
		//				return;
		//		}

		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA] = idCatA;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBMaterialEditWidget::on_toolButtonSelectCatB_clicked() {
	// get EPD edit dialog from mainwindow
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();
	unsigned int idCatB = editDialog->select(m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryB], false, "B", 4);
	if (idCatB != VICUS::INVALID_ID && idCatB != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryB]) {
		VICUS::EpdDataset * epd = const_cast<VICUS::EpdDataset *>(m_db->m_epdDatasets[idCatB]);

		//		if(epd->m_module != VICUS::EpdDataset::M_B1 &&
		//				epd->m_module != VICUS::EpdDataset::M_B2  &&
		//				epd->m_module != VICUS::EpdDataset::M_B3  &&
		//				epd->m_module != VICUS::EpdDataset::M_B4  &&
		//				epd->m_module != VICUS::EpdDataset::M_B5  &&
		//				epd->m_module != VICUS::EpdDataset::M_B6  &&
		//				epd->m_module != VICUS::EpdDataset::M_B7 )
		//		{
		//			std::string keyword = VICUS::KeywordList::Keyword("EPDDataset::Module", epd->m_module);
		//			if (QMessageBox::warning(this, tr("Material EPD Assignement"),
		//										   tr("You want to assign an EPD with Module '%1' to Category B.\n"
		//											  "Are you sure to do this?").arg(QString::fromStdString(keyword)),
		//										   QMessageBox::Yes | QMessageBox::Discard) == QMessageBox::Discard)
		//				return;
		//		}

		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryB] = idCatB;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBMaterialEditWidget::on_toolButtonSelectCatC_clicked() {
	// get EPD edit dialog from mainwindow
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();
	unsigned int idCatC = editDialog->select(m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryC], false, "C", 4); // we do not want to reset the model all the time

	if (idCatC != VICUS::INVALID_ID && idCatC != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryC]) {
		VICUS::EpdDataset * epd = const_cast<VICUS::EpdDataset *>(m_db->m_epdDatasets[idCatC]);

		//		if(epd->m_module != VICUS::EpdDataset::M_C1 &&
		//				epd->m_module != VICUS::EpdDataset::M_C2 &&
		//				epd->m_module != VICUS::EpdDataset::M_C2_C4 &&
		//				epd->m_module != VICUS::EpdDataset::M_C2_C3 &&
		//				epd->m_module != VICUS::EpdDataset::M_C3 &&
		//				epd->m_module != VICUS::EpdDataset::M_C3_C4 &&
		//				epd->m_module != VICUS::EpdDataset::M_C4 )
		//		{
		//			std::string keyword = VICUS::KeywordList::Keyword("EPDDataset::Module", epd->m_module);
		//			if (QMessageBox::warning(this, tr("Material EPD Assignement"),
		//										   tr("You want to assign an EPD with Module '%1' to Category C.\n"
		//											  "Are you sure to do this?").arg(QString::fromStdString(keyword)),
		//										   QMessageBox::Yes | QMessageBox::Discard) == QMessageBox::Discard)
		//				return;
		//		}


		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryC] = idCatC;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBMaterialEditWidget::on_toolButtonSelectCatD_clicked() {
	// get EPD edit dialog from mainwindow
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();
	unsigned int idCatD = editDialog->select(m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD], false, "D", 4);
	if (idCatD != VICUS::INVALID_ID && idCatD != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD]) {
		VICUS::EpdDataset * epd = const_cast<VICUS::EpdDataset *>(m_db->m_epdDatasets[idCatD]);

		//		if(epd->m_module != VICUS::EpdDataset::M_D )
		//		{
		//			std::string keyword = VICUS::KeywordList::Keyword("EPDDataset::Module", epd->m_module);
		//			if (QMessageBox::warning(this, tr("Material EPD Assignement"),
		//										   tr("You want to assign an EPD with Module '%1' to Category D.\n"
		//											  "Are you sure to do this?").arg(QString::fromStdString(keyword)),
		//										   QMessageBox::Yes | QMessageBox::Discard) == QMessageBox::Discard)
		//				return;
		//		}


		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD] = idCatD;
		modelModify();
	}
	updateInput((int)m_current->m_id);
}


void SVDBMaterialEditWidget::on_pushButton_clicked() {
	SVDatabaseEditDialog * editDialog = SVMainWindow::instance().dbEpdEditDialog();
	unsigned int idCat = editDialog->select(m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD], false);
	if (idCat != VICUS::INVALID_ID && idCat != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA]) {
		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryA] = idCat;
	}
	if (idCat != VICUS::INVALID_ID && idCat != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryB]) {
		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryB] = idCat;
	}
	if (idCat != VICUS::INVALID_ID && idCat != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryC]) {
		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryC] = idCat;
	}
	if (idCat != VICUS::INVALID_ID && idCat != m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD]) {
		m_current->m_epdCategorySet.m_idCategory[VICUS::EpdCategorySet::C_IDCategoryD] = idCat;
	}
	modelModify();
	updateInput((int)m_current->m_id);
}

