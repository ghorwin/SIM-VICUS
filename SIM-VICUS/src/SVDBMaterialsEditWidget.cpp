#include "SVDBMaterialsEditWidget.h"
#include "ui_SVDBMaterialsEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMaterialTransfer.h"

#include <NANDRAD_KeywordList.h>

SVDBMaterialsEditWidget::SVDBMaterialsEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBMaterialsEditWidget)
{
	m_ui->setupUi(this);
	QSet<QtExt::MaterialBase::parameter_t> visibleParams;

	//here select all configuration for visible params
	visibleParams << QtExt::MaterialBase::MC_ID;
	visibleParams << QtExt::MaterialBase::MC_NAME;
	visibleParams << QtExt::MaterialBase::MC_CATEGORY;
	//visibleParams << QtExt::MaterialBase::MC_LAMBDA;
	visibleParams << QtExt::MaterialBase::MC_RHO;
	//visibleParams << QtExt::MaterialBase::MC_CET;

	///TODO Dirk Einstellungsfenster bauen um sich die Tabellenspalten selbst einzustellen

	SVSettings::instance().readDatabase();
	m_dbMat = &SVSettings::instance().m_dbOpaqueMaterials;

	m_ui->widgetMaterialsDB->setup(visibleParams,ORG_NAME, PROGRAM_NAME, "Material Editor");

	std::vector<QtExt::MaterialBase*> mats;
	for(auto & e : *m_dbMat)
		mats.push_back(new SVMaterialTransfer(e.second));

	m_ui->widgetMaterialsDB->setMaterials(mats);

	//old style
	//	SLOT(onMaterialSelected(int));

	//new style -> Andreas fragen
	//	&onMaterialSelected
	connect(m_ui->widgetMaterialsDB, SIGNAL(materialSelected(int)),this, SLOT(onMaterialSelected(int)));
	connect(m_ui->lineEditDensity, SIGNAL(editingFinishedSuccessfully()),this, SLOT(editingFinishedSuccessfully()));
	connect(m_ui->lineEditConductivity, SIGNAL(editingFinishedSuccessfully()),this, SLOT(editingFinishedSuccessfully()));
	connect(m_ui->lineEditSpecHeatCapacity, SIGNAL(editingFinishedSuccessfully()),this, SLOT(editingFinishedSuccessfully()));

	//setup line edits
	m_ui->lineEditConductivity->setup(0,500,"Thermal Conductivity",false,true);
	m_ui->lineEditDensity->setup(1, 10000,"Thermal Density",true,true);
	m_ui->lineEditSpecHeatCapacity->setup(200, 4500,"Specific Heat Capacity",true,true);

	//set names
	m_ui->labelDensity->setText(tr("Density [kg/m3]"));
	m_ui->labelConductivity->setText(tr("Conductivity [W/mK]"));
	m_ui->labelSpecHeatCapacity->setText(tr("Spec. Heat Capacity [J/kgK]"));

	//init buttons
	m_ui->toolButtonRemove->setEnabled(false);
	m_ui->toolButtonCopy->setEnabled(false);

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
		m_ui->toolButtonRemove->setEnabled(false);
		m_ui->toolButtonCopy->setEnabled(false);
	}
	else {
		std::map<unsigned int, VICUS::Material>::const_iterator matIt = m_dbMat->find(id);
		Q_ASSERT(matIt != m_dbMat->end());
		m_ui->lineEditDensity->setValue(matIt->second.m_para[VICUS::Material::P_Density].get_value("kg/m3"));
		m_ui->lineEditConductivity->setValue(matIt->second.m_para[VICUS::Material::P_Conductivity].get_value("W/mK"));
		m_ui->lineEditSpecHeatCapacity->setValue(matIt->second.m_para[VICUS::Material::P_HeatCapacity].get_value("J/kgK"));

		/// TODO Andreas woher weiß ich wann es ein User Material ist und wann nicht?
		//m_ui->widgetMaterialsDB->
		if(false)
			m_ui->toolButtonRemove->setEnabled(true);

		m_ui->toolButtonCopy->setEnabled(true);

//		m_selectedMaterial = matIt->first;
//		m_toolButtonCopy->setEnabled(m_selectedMaterial > -1);

//		bool enabled = m_selectedMaterial > 10000;

//		m_toolButtonRemove->setEnabled(enabled);
	}

}

void SVDBMaterialsEditWidget::edit()
{
	// update UI to current state of database in settings


	// finally show the widget
	show();
}

void SVDBMaterialsEditWidget::closeEvent(QCloseEvent * event)
{
	SVSettings::instance().writeDatabase();
	QWidget::closeEvent(event);
}

void SVDBMaterialsEditWidget::editingFinishedSuccessfully()
{
	int id = m_actualId;
	if(m_ui->lineEditDensity->isValid()){
//		NANDRAD::KeywordList::setParameter(m_dbMat->find(id)->second.m_para, "VICUS::Material::para_t",
//										   VICUS::Material::P_Density, m_ui->lineEditDensity->value());
		m_dbMat->find(id)->second.m_para[VICUS::Material::P_Density].set("Density", m_ui->lineEditDensity->value(), "kg/m3");
	}
	if(m_ui->lineEditConductivity->isValid()){
		m_dbMat->find(id)->second.m_para[VICUS::Material::P_Conductivity].set("Conductivity", m_ui->lineEditConductivity->value(), "W/mK");
	}
	if(m_ui->lineEditSpecHeatCapacity->isValid()){
		m_dbMat->find(id)->second.m_para[VICUS::Material::P_HeatCapacity].set("HeatCapacity", m_ui->lineEditSpecHeatCapacity->value(), "J/kgK");
	}
}


void SVDBMaterialsEditWidget::on_toolButtonAdd_clicked()
{
	//getFreeId<VICUS::Material>(*m_dbMat, 1000000);
	std::map<unsigned int, VICUS::Material> & db = *m_dbMat;

	unsigned int id = SVSettings::firstFreeId(db, 1000000);//getFreeId<VICUS::Material>(*m_dbMat, 1000000);
	VICUS::Material newMat(id, "unnamed", 1, 1000, 840);

	db[id] = newMat;

	std::vector<QtExt::MaterialBase*> mats;
	for(auto & e : db)
		mats.push_back(new SVMaterialTransfer(e.second));

	m_ui->widgetMaterialsDB->setMaterials(mats);

	//select new material for user
}

void SVDBMaterialsEditWidget::on_toolButtonCopy_clicked()
{

}

void SVDBMaterialsEditWidget::on_lineEditConductivity_editingFinished()
{
	if ( m_ui->lineEditConductivity->isValid() ){
		// setze werte in datenbank
	}
}


