#include "SVDBMaterialsEditWidget.h"
#include "ui_SVDBMaterialsEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMaterialTransfer.h"

#include <NANDRAD_KeywordList.h>

///TODO Andreas wie kann die Table nach einem editieren wieder geupdatet werden. Die Werte der Materialien werden
/// gespeichert aber die Daten in der Tabelle werden nicht erneuert.
/// TODO Heiko wie bin ich die anderen Sachen ein ohne das es zu einem Absturz kommt
/// Wie können wir beim Spaltenbreite editieren verhindern dass die anderen Spalten "verzogen" werden.

SVDBMaterialsEditWidget::SVDBMaterialsEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBMaterialsEditWidget)
{
	m_ui->setupUi(this);
#if 0
	QSet<QtExt::MaterialBase::parameter_t> visibleParams;

	//here select all configuration for visible params
	visibleParams << QtExt::MaterialBase::MC_ID;
	visibleParams << QtExt::MaterialBase::MC_NAME;
	visibleParams << QtExt::MaterialBase::MC_CATEGORY;
	//visibleParams << QtExt::MaterialBase::MC_LAMBDA;
	visibleParams << QtExt::MaterialBase::MC_RHO;
	//visibleParams << QtExt::MaterialBase::MC_CET;

	///TODO Dirk Einstellungsfenster bauen um sich die Tabellenspalten selbst einzustellen

//	m_dbMat = &SVSettings::instance().m_dbOpaqueMaterials;

//	m_ui->widgetMaterialsDB->setup(visibleParams,ORG_NAME, PROGRAM_NAME, "Material Editor");

	std::vector<QtExt::MaterialBase*> mats;
	for(auto & e : *m_dbMat)
		mats.push_back(new SVMaterialTransfer(e.second));

//	m_ui->widgetMaterialsDB->setMaterials(mats);

	//old style
	//	SLOT(onMaterialSelected(int));

	//new style -> Andreas fragen
	//	&onMaterialSelected
	connect(m_ui->widgetMaterialsDB, SIGNAL(materialSelected(int)),this, SLOT(onMaterialSelected(int)));
	connect(m_ui->lineEditDensity, SIGNAL(editingFinishedSuccessfully()),this, SLOT(editingFinishedSuccessfully()));
	connect(m_ui->lineEditConductivity, SIGNAL(editingFinishedSuccessfully()),this, SLOT(editingFinishedSuccessfully()));
	connect(m_ui->lineEditSpecHeatCapacity, SIGNAL(editingFinishedSuccessfully()),this, SLOT(editingFinishedSuccessfully()));

	//setup line edits
	m_ui->lineEditConductivity->setup(0,500,tr("Thermal Conductivity. Value between >0 and 500."),false,true);
	m_ui->lineEditDensity->setup(1, 10000,tr("Thermal Density. Value between 1 and 10000."),true,true);
	m_ui->lineEditSpecHeatCapacity->setup(200, 4500,tr("Specific Heat Capacity. Value between 200 and 4500."),true,true);

	//set names
	m_ui->labelDensity->setText(tr("Density [kg/m3]"));
	m_ui->labelConductivity->setText(tr("Conductivity [W/mK]"));
	m_ui->labelSpecHeatCapacity->setText(tr("Spec. Heat Capacity [J/kgK]"));

	//init buttons
	m_ui->toolButtonRemove->setEnabled(false);
	m_ui->toolButtonCopy->setEnabled(false);

	//init comboboxes
	m_ui->comboBoxLanguage->addItem(QString("EN"), 0);
	m_ui->comboBoxLanguage->addItem(QString("DE"), 1);
	m_ui->comboBoxLanguage->setCurrentIndex(0);

	for (unsigned int i=0; i< VICUS::Material::Category::NUM_MC; ++i) {
		m_ui->comboBoxCategory->addItem(VICUS::Material::categoryToString( (VICUS::Material::Category)(i) ) );
	}

	//init line edit
	m_ui->lineEditDisplayName->setText("");
	update(-1);
#endif
}

void SVDBMaterialsEditWidget::onMaterialSelected(int id){
	//refresh labels/linesedit ... in right window
	//pointer neu setzen
	//
	m_actualId = id;
	update(id);
}

SVDBMaterialsEditWidget::~SVDBMaterialsEditWidget() {
	delete m_ui;
}

void SVDBMaterialsEditWidget::update(int id)
{
	if(id == -1){
		//disable buttons
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);

		//disable and clear input controls
		m_ui->lineEditDensity->setReadOnly(true);
		m_ui->lineEditDensity->setText("");
		m_ui->lineEditConductivity->setReadOnly(true);
		m_ui->lineEditConductivity->setText("");
		m_ui->lineEditSpecHeatCapacity->setReadOnly(true);
		m_ui->lineEditSpecHeatCapacity->setText("");
		m_ui->lineEditDisplayName->setReadOnly(true);
		m_ui->lineEditDisplayName->setText("");
		m_ui->comboBoxCategory->setEnabled(false);
		m_ui->comboBoxLanguage->setEnabled(false);
		m_ui->pushButtonOpaqueMaterialColor->setEnabled(false);
	}
	else {
		//enable controls
		m_ui->lineEditDisplayName->setReadOnly(false);
		m_ui->lineEditDensity->setReadOnly(false);
		m_ui->lineEditConductivity->setReadOnly(false);
		m_ui->lineEditSpecHeatCapacity->setReadOnly(false);
		m_ui->comboBoxCategory->setEnabled(true);
		m_ui->comboBoxLanguage->setEnabled(true);
		m_ui->pushButtonOpaqueMaterialColor->setEnabled(true);



		std::map<unsigned int, VICUS::Material>::const_iterator matIt = m_dbMat->find(id);
		Q_ASSERT(matIt != m_dbMat->end());
		const VICUS::Material &mat = matIt->second;
		m_ui->lineEditDensity->setValue(mat.m_para[VICUS::Material::P_Density].get_value("kg/m3"));
		m_ui->lineEditConductivity->setValue(mat.m_para[VICUS::Material::P_Conductivity].get_value("W/mK"));
		m_ui->lineEditSpecHeatCapacity->setValue(mat.m_para[VICUS::Material::P_HeatCapacity].get_value("J/kgK"));
		m_ui->comboBoxCategory->setCurrentIndex(mat.m_category);
		///TODO Dirk Einbauen des Multilanguage string --> Andreas fragen


		/// TODO Andreas woher weiß ich wann es ein User Material ist und wann nicht?
		//m_ui->widgetMaterialsDB->
		if(false)
			m_ui->toolButtonRemove->setEnabled(true);

		m_ui->toolButtonCopy->setEnabled(true);

//		m_selectedMaterial = matIt->first;
//		m_toolButtonCopy->setEnabled(m_selectedMaterial > -1);

		if(!mat.m_builtIn)
			m_ui->toolButtonRemove->setEnabled(true);

	}

}


void SVDBMaterialsEditWidget::edit() {
	// update UI to current state of database in settings


	// finally show the widget
	show();
}


void SVDBMaterialsEditWidget::closeEvent(QCloseEvent * event) {
	QWidget::closeEvent(event);
}

void SVDBMaterialsEditWidget::editingFinishedSuccessfully()
{
	int id = m_actualId;
	VICUS::Material &mat = m_dbMat->find(id)->second;
	if(m_ui->lineEditDensity->isValid()){
//		NANDRAD::KeywordList::setParameter(m_dbMat->find(id)->second.m_para, "VICUS::Material::para_t",
//										   VICUS::Material::P_Density, m_ui->lineEditDensity->value());
		mat.m_para[VICUS::Material::P_Density].set("Density", m_ui->lineEditDensity->value(), "kg/m3");
	}
	if(m_ui->lineEditConductivity->isValid()){
		mat.m_para[VICUS::Material::P_Conductivity].set("Conductivity", m_ui->lineEditConductivity->value(), "W/mK");
	}
	if(m_ui->lineEditSpecHeatCapacity->isValid()){
		mat.m_para[VICUS::Material::P_HeatCapacity].set("HeatCapacity", m_ui->lineEditSpecHeatCapacity->value(), "J/kgK");
	}
	/// TODO : Dirk name to multilanguage string -> fix language ID
	mat.m_displayName.setString(m_ui->lineEditDisplayName->text().toStdString(), "en");
	mat.m_category = (VICUS::Material::Category)(m_ui->comboBoxCategory->currentIndex());
}



void SVDBMaterialsEditWidget::on_lineEditConductivity_editingFinished()
{
	if ( m_ui->lineEditConductivity->isValid() ){
		// setze werte in datenbank
	}
}


