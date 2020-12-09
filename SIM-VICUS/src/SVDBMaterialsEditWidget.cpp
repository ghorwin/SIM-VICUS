#include "SVDBMaterialsEditWidget.h"
#include "ui_SVDBMaterialsEditWidget.h"

#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMaterialTransfer.h"

SVDBMaterialsEditWidget::SVDBMaterialsEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBMaterialsEditWidget)
{
	m_ui->setupUi(this);
	QSet<QtExt::MaterialBase::parameter_t> visibleParams;

	//here select all configuration for visible params
	visibleParams << QtExt::MaterialBase::MC_ID;
	visibleParams << QtExt::MaterialBase::MC_NAME;


	m_ui->widgetMaterialsDB->setup(visibleParams,ORG_NAME, PROGRAM_NAME, "Material Editor");
	std::vector<QtExt::MaterialBase*> mats;

	for(auto & e : SVSettings::instance().m_dbOpaqueMaterials)
		mats.push_back(new SVMaterialTransfer(e.second));

	m_ui->widgetMaterialsDB->setMaterials(mats);

	//old style
	//	SLOT(onMaterialSelected(int));

	//new style -> Andreas fragen
	//	&onMaterialSelected
	connect(m_ui->widgetMaterialsDB, SIGNAL(materialSelected(int)),this, SLOT(onMaterialSelected(int)));

}

void SVDBMaterialsEditWidget::onMaterialSelected(int id){
	//refresh labels/linesedit ... in right window
	//pointer neu setzen
	//
	update(id);
}

SVDBMaterialsEditWidget::~SVDBMaterialsEditWidget() {
	delete m_ui;
}

void SVDBMaterialsEditWidget::update(int id)
{
	//hier alles neu setzen
}

void SVDBMaterialsEditWidget::edit()
{
	// update UI to current state of database in settings


	// finally show the widget
	show();
}


void SVDBMaterialsEditWidget::on_toolButtonAdd_clicked()
{

}

void SVDBMaterialsEditWidget::on_toolButtonCopy_clicked()
{

}
