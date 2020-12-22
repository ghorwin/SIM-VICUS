#include "SVDBMaterialEditWidget.h"
#include "ui_SVDBMaterialEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVDBMaterialTableModel.h"

SVDBMaterialEditWidget::SVDBMaterialEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBMaterialEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption(tr("Material identification name"));
	m_ui->lineEditDataSource->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditDataSource->setDialog3Caption(tr("Data source information"));
	m_ui->lineEditManufacturer->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditManufacturer->setDialog3Caption(tr("Manufacturer name"));
	m_ui->lineEditNotes->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditNotes->setDialog3Caption(tr("Notes"));

	m_ui->lineEditDensity->setup(1, 10000, tr("Density"), true, true);
	m_ui->lineEditConductivity->setup(0.001, 500, tr("Thermal conductivity"), true, true);
	m_ui->lineEditSpecHeatCapacity->setup(200, 5000, tr("Specific heat capacity"), true, true);

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxCategory->blockSignals(true);
	for (int i=0; i<VICUS::Material::NUM_MC; ++i)
		m_ui->comboBoxCategory->addItem(VICUS::KeywordListQt::Keyword("Material::Category", i), i);
	m_ui->comboBoxCategory->blockSignals(false);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBMaterialEditWidget::~SVDBMaterialEditWidget() {
	delete m_ui;
}


void SVDBMaterialEditWidget::setup(SVDatabase * db, SVDBMaterialTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
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

	// now update the GUI controls

	m_ui->lineEditDensity->setValue(mat->m_para[VICUS::Material::P_Density].value);
	m_ui->lineEditConductivity->setValue(mat->m_para[VICUS::Material::P_Conductivity].value);
	m_ui->lineEditSpecHeatCapacity->setValue(mat->m_para[VICUS::Material::P_HeatCapacity].value);
	m_ui->lineEditName->setString(mat->m_displayName);
	m_ui->lineEditDataSource->setString(mat->m_dataSource);
	m_ui->lineEditManufacturer->setString(mat->m_manufacturer);
	m_ui->lineEditNotes->setString(mat->m_notes);
	m_ui->comboBoxCategory->setCurrentIndex(mat->m_category);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (mat->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setEnabled(isEditable);
	m_ui->lineEditDataSource->setEnabled(isEditable);
	m_ui->lineEditManufacturer->setEnabled(isEditable);
	m_ui->lineEditNotes->setEnabled(isEditable);
	m_ui->lineEditDensity->setEnabled(isEditable);
	m_ui->lineEditConductivity->setEnabled(isEditable);
	m_ui->lineEditSpecHeatCapacity->setEnabled(isEditable);
	m_ui->comboBoxCategory->setEnabled(isEditable);

}


void SVDBMaterialEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_materials.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}
void SVDBMaterialEditWidget::on_lineEditDataSource_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_dataSource != m_ui->lineEditDataSource->string()) {
		m_current->m_dataSource = m_ui->lineEditDataSource->string();
		m_db->m_materials.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}

void SVDBMaterialEditWidget::on_lineEditManufacturer_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_manufacturer != m_ui->lineEditManufacturer->string()) {
		m_current->m_manufacturer = m_ui->lineEditManufacturer->string();
		m_db->m_materials.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}

void SVDBMaterialEditWidget::on_lineEditNotes_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_notes != m_ui->lineEditNotes->string()) {
		m_current->m_notes = m_ui->lineEditNotes->string();
		m_db->m_materials.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
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
			m_db->m_materials.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
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
			m_db->m_materials.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
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
			m_db->m_materials.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit tableDataChanged();
		}
	}
}

void SVDBMaterialEditWidget::on_comboBoxCategory_currentIndexChanged(int index){
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_category)
	{
		m_current->m_category = static_cast<VICUS::Material::Category>(index);
		m_db->m_materials.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}


