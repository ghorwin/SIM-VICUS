#include "SVDialogHydraulicComponents.h"
#include "ui_SVDialogHydraulicComponents.h"

#include "SVProjectHandler.h"
#include "SVUndoModifyNetworkHydraulicComponent.h"
#include "SVUndoDeleteNetworkHydraulicComponent.h"
#include "SVHydraulicComponentParameterModel.h"

#include <VICUS_Project.h>

#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordList.h>


SVDialogHydraulicComponents::SVDialogHydraulicComponents(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDialogHydraulicComponents)
{
	m_ui->setupUi(this);

	setWindowTitle("Hydraulic Components");

	m_ui->tableViewComponentParams->horizontalHeader()->setVisible(true);

	// populate combobox components
	m_mapComponents.clear();
	for (unsigned int i=0; i<NANDRAD::HydraulicNetworkComponent::NUM_MT; ++i){
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::ModelType",
														NANDRAD::HydraulicNetworkComponent::ModelType(i)),
														NANDRAD::HydraulicNetworkComponent::ModelType(i));
	}
	m_ui->comboBoxComponentType->clear();
	m_ui->comboBoxComponentType->addItems(m_mapComponents.keys());

	// populate combobox heat exchange type
	m_mapHeatExchangeType.clear();
	m_mapHeatExchangeType.insert("None", NANDRAD::HydraulicNetworkComponent::NUM_HT);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::HeatExchangeType",
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant),
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::HeatExchangeType",
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile),
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::HeatExchangeType",
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature),
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::HeatExchangeType",
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature),
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature);
	m_ui->comboBoxHeatExchangeType->clear();
	m_ui->comboBoxHeatExchangeType->addItems(m_mapHeatExchangeType.keys());
	m_ui->comboBoxHeatExchangeType->setCurrentText(m_mapHeatExchangeType.key(NANDRAD::HydraulicNetworkComponent::NUM_HT));
}

SVDialogHydraulicComponents::~SVDialogHydraulicComponents()
{
	delete m_ui;
}


int SVDialogHydraulicComponents::edit(const unsigned int networkId, const unsigned int currentComponentId)
{
	const VICUS::Project &p = project();

	// setup combobox networks
	m_mapNetworks.clear();
	for (auto it = p.m_geometricNetworks.begin(); it!=p.m_geometricNetworks.end(); ++it)
		m_mapNetworks.insert(QString::fromStdString(it->m_name), it->m_id);
	m_ui->comboBoxNetwork->clear();
	m_ui->comboBoxNetwork->addItems(QStringList(m_mapNetworks.keys()));
	if (p.element(p.m_geometricNetworks, networkId) != nullptr)
		m_ui->comboBoxNetwork->setCurrentText(m_mapNetworks.key(networkId));

	populateListWidget(currentComponentId);

	return exec();
}

unsigned int SVDialogHydraulicComponents::currentComponentId()
{
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	if (item != nullptr)
		return item->id();
	else
		return 0;
}


const VICUS::Network * SVDialogHydraulicComponents::currentNetwork()
{
	const VICUS::Project &p = project();
	return p.element(p.m_geometricNetworks, m_mapNetworks.value(m_ui->comboBoxNetwork->currentText()));
}

void SVDialogHydraulicComponents::updateComponent()
{
	const VICUS::Network * network = currentNetwork();
	if (network->m_hydraulicComponents.empty())
		return;
	HydraulicComponentItem * item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	if (item == nullptr)
		return;
	const NANDRAD::HydraulicNetworkComponent * component = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(component != nullptr);

	m_ui->labelComponentId->setText(tr("%1").arg(component->m_id));
	m_ui->lineEditName->setText(QString::fromStdString(component->m_displayName));
	m_ui->comboBoxComponentType->setCurrentText(m_mapComponents.key(component->m_modelType));
	m_ui->comboBoxHeatExchangeType->setEnabled(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(component->m_modelType));
	m_ui->comboBoxHeatExchangeType->setCurrentText(m_mapHeatExchangeType.key(component->m_heatExchangeType));

	item->setText(QString::fromStdString(component->m_displayName));

	updateTableView();
}

void SVDialogHydraulicComponents::updateTableView()
{
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	const VICUS::Network * network = currentNetwork();
	const NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(comp != nullptr);

	m_componentParModel = new SVHydraulicComponentParameterModel(this);
	connect(m_componentParModel, SIGNAL(editCompleted()), this, SLOT(on_componentParModel_editCompleted()));
	m_componentParModel->setComponent(*comp);
	m_ui->tableViewComponentParams->setModel(m_componentParModel);
	m_ui->tableViewComponentParams->show();
}



void SVDialogHydraulicComponents::modifyComponent() {
	const VICUS::Network * network = currentNetwork();
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	const NANDRAD::HydraulicNetworkComponent *tmpComp = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(tmpComp != nullptr);
	NANDRAD::HydraulicNetworkComponent component = *tmpComp;

	component.m_displayName = m_ui->lineEditName->text().toStdString();
	component.m_modelType = NANDRAD::HydraulicNetworkComponent::ModelType(
				m_mapComponents.value(m_ui->comboBoxComponentType->currentText()));
	component.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::HeatExchangeType(
				m_mapHeatExchangeType.value(m_ui->comboBoxHeatExchangeType->currentText()));

	if (component.m_modelType == NANDRAD::HydraulicNetworkComponent::NUM_MT)
		return;

	// get index of modified component
	unsigned int compIndex = std::distance(&network->m_hydraulicComponents.front(), tmpComp);
	SVUndoModifyNetworkHydraulicComponent *undo = new SVUndoModifyNetworkHydraulicComponent(
				"Modified network component", network->m_id, compIndex, component);
	undo->push();

	updateComponent();
}


void SVDialogHydraulicComponents::modifyTableView() {
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	const VICUS::Network * network = currentNetwork();
	const NANDRAD::HydraulicNetworkComponent *tmpComp = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(tmpComp != nullptr);
	NANDRAD::HydraulicNetworkComponent component = *tmpComp;
	m_componentParModel->getComponentParameter(component.m_para);
	unsigned int compIndex = std::distance(&network->m_hydraulicComponents.front(), tmpComp);
	SVUndoModifyNetworkHydraulicComponent *undo = new SVUndoModifyNetworkHydraulicComponent(
				"Modified network component", network->m_id, compIndex, component);
	undo->push();
}


void SVDialogHydraulicComponents::addComponent(const std::string & name, const NANDRAD::HydraulicNetworkComponent * comp)
{
	const VICUS::Project &p = project();
	const VICUS::Network * network = currentNetwork();
	unsigned int id = p.uniqueId(network->m_hydraulicComponents);

	// new list widget item
	HydraulicComponentItem * item = new HydraulicComponentItem(QString::fromStdString(name));
	item->setId(id);
	m_ui->listWidget->addItem(item);
	m_ui->listWidget->setCurrentItem(item);

	// new component
	NANDRAD::HydraulicNetworkComponent component;
	if (comp != nullptr)
		component = *comp;
	component.m_displayName = name;
	component.m_id = id;

	// TODO : Hauke, add and use new undo action "SVUndoAddNetworkHydraulicComponent"
//	SVUndoModifyNetworkHydraulicComponent *undo = new SVUndoModifyNetworkHydraulicComponent("modified comp",
//																							network->m_id, component);
//	undo->push();

	updateComponent();
}

void SVDialogHydraulicComponents::on_componentParModel_editCompleted()
{
	modifyTableView();
}


void SVDialogHydraulicComponents::on_lineEditName_editingFinished()
{
	modifyComponent();
}

void SVDialogHydraulicComponents::on_comboBoxComponentType_activated(const QString &arg1)
{
	modifyComponent();
}

void SVDialogHydraulicComponents::on_comboBoxHeatExchangeType_activated(const QString &arg1)
{
	modifyComponent();
}

void SVDialogHydraulicComponents::on_toolButtonAdd_clicked()
{
	addComponent("new component", nullptr);
}

void SVDialogHydraulicComponents::on_toolButtonCopy_clicked()
{
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	if (item == nullptr)
		return;
	const VICUS::Network * network = currentNetwork();
	const NANDRAD::HydraulicNetworkComponent * tmpComp = VICUS::Project::element(network->m_hydraulicComponents, item->id());

	addComponent(tmpComp->m_displayName + " copy", tmpComp);
}

void SVDialogHydraulicComponents::on_toolButtonRemove_clicked()
{
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	if (item == nullptr)
		return;
	const VICUS::Network * network = currentNetwork();
	const NANDRAD::HydraulicNetworkComponent * comp = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(comp != nullptr);

	delete item;
	SVUndoDeleteNetworkHydraulicComponent *undo = new SVUndoDeleteNetworkHydraulicComponent("delete comp", network->m_id, *comp);
	undo->push();
}

void SVDialogHydraulicComponents::on_comboBoxNetwork_activated(int index)
{
	populateListWidget();
}

void SVDialogHydraulicComponents::on_listWidget_itemClicked(QListWidgetItem *item)
{
	updateComponent();
}

void SVDialogHydraulicComponents::populateListWidget(unsigned int componentId)
{
	const VICUS::Network * network = currentNetwork();
	m_ui->listWidget->clear();
	for (const NANDRAD::HydraulicNetworkComponent &comp: network->m_hydraulicComponents){
		HydraulicComponentItem * item = new HydraulicComponentItem(QString::fromStdString(comp.m_displayName));
		item->setId(comp.m_id);
		m_ui->listWidget->addItem(item);
		if (item->id() == componentId)
			m_ui->listWidget->setCurrentItem(item);
	}
	updateComponent();
}



// *** HydraulicComponentItem ***

HydraulicComponentItem::HydraulicComponentItem(const QString & name, QListWidget * parent, int type):
	QListWidgetItem(name, parent, type)
{
}

unsigned HydraulicComponentItem::id() const
{
	return m_id;
}

void HydraulicComponentItem::setId(const unsigned &id)
{
	m_id = id;
}


