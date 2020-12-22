#include "SVDBMaterialEditWidget.h"
#include "ui_SVDBMaterialEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"

#include <VICUS_KeywordList.h>

SVDBMaterialEditWidget::SVDBMaterialEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBMaterialEditWidget)
{
	m_ui->setupUi(this);


	m_ui->lineEditDensity->setup(1, 10000, tr("Density"), true, true);
	m_ui->lineEditConductivity->setup(0.001, 500, tr("Thermal conductivity"), true, true);
	m_ui->lineEditSpecHeatCapacity->setup(200, 5000, tr("Specific heat capacity"), true, true);

	QStringList sl;
	sl << "Coating";
	sl << "Plaster";
	sl << "Bricks";
	sl << "NaturalStones";
	sl << "Cementitious";
	sl << "Insulations";
	sl << "BuildingBoards";
	sl << "Woodbased";
	sl << "NaturalMaterials";
	sl << "Soils";
	sl << "CladdingSystems";
	sl << "Foils";
	sl << "Miscellaneous";
	m_ui->comboBoxCategory->addItems(sl);

	sl.clear();
	sl << "ENG";
	sl << "DE";
	m_ui->comboBoxLanguage->addItems(sl);

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
		m_ui->lineEditDisplayName->setText("");

		return;
	}
	m_ui->tabWidgetThermal->setEnabled(true);

	VICUS::Material * mat = const_cast<VICUS::Material *>(m_db->m_materials[(unsigned int)id]);
	m_current = mat;

	// now update the GUI controls

	m_ui->lineEditDensity->setValue(mat->m_para[VICUS::Material::P_Density].value);
	m_ui->lineEditConductivity->setValue(mat->m_para[VICUS::Material::P_Conductivity].value);
	m_ui->lineEditSpecHeatCapacity->setValue(mat->m_para[VICUS::Material::P_HeatCapacity].value);
	m_ui->comboBoxCategory->setCurrentIndex(mat->m_category);

	// TODO : Dirk, andere Editfelder implementieren
	IBK::MultiLanguageString mStr;
	//if(mStr.hasLanguage()

}


void SVDBMaterialEditWidget::on_lineEditConductivity_editingFinished() {
	if ( m_ui->lineEditConductivity->isValid() ){
		// setze werte in datenbank
	}
}


