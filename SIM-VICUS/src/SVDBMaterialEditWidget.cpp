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
	m_ui->comboBoxCategory->setCurrentIndex(mat->m_category);

	// for built-ins, disable editing/make read-only
	if (mat->m_builtIn) {

	}

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


