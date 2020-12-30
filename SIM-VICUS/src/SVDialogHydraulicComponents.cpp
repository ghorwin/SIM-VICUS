#include "SVDialogHydraulicComponents.h"
#include "ui_SVDialogHydraulicComponents.h"

#include "SVProjectHandler.h"
#include "SVUndoNetworkHydraulicComponent.h"

#include "VICUS_Project.h"

#include "NANDRAD_HydraulicNetworkComponent.h"
#include "NANDRAD_KeywordList.h"

#include "QtExt_Locale.h"


SVDialogHydraulicComponents::SVDialogHydraulicComponents(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVDialogHydraulicComponents)
{
	m_ui->setupUi(this);

	m_ui->tableViewComponentParams->horizontalHeader()->setVisible(true);

	// populate combobox components
	m_mapComponents.clear();
	m_mapComponents.insert("<None>", NANDRAD::HydraulicNetworkComponent::NUM_MT);
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_Radiator),
														NANDRAD::HydraulicNetworkComponent::MT_Radiator);
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger),
														NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger);
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_HeatPump),
														NANDRAD::HydraulicNetworkComponent::MT_HeatPump);
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel),
														NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel);
	m_ui->comboBoxComponentType->clear();
	m_ui->comboBoxComponentType->addItems(m_mapComponents.keys());
	m_ui->comboBoxComponentType->setCurrentText(m_mapComponents.key(NANDRAD::HydraulicNetworkComponent::NUM_MT));

	// setup combobox heat exchange type
	m_mapHeatExchangeType.clear();
	m_mapHeatExchangeType.insert("None", NANDRAD::HydraulicNetworkComponent::NUM_HT);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant),
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile),
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature),
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
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


void SVDialogHydraulicComponents::edit(const unsigned int networkId)
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

	const VICUS::Network * network = currentNetwork();

	// populate list widget
	m_ui->listWidget->clear();
	for (const NANDRAD::HydraulicNetworkComponent &comp: network->m_hydraulicComponents){
		HydraulicComponentItem * item = new HydraulicComponentItem(QString::fromStdString(comp.m_displayName));
		item->setName(QString::fromStdString(comp.m_displayName));
		item->setId(comp.m_id);
		m_ui->listWidget->addItem(item);
	}

	exec();
}


const VICUS::Network * SVDialogHydraulicComponents::currentNetwork()
{
	const VICUS::Project &p = project();
	return p.element(p.m_geometricNetworks, m_mapNetworks.value(m_ui->comboBoxNetwork->currentText()));
}

void SVDialogHydraulicComponents::updateComponent()
{
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	const VICUS::Network * network = currentNetwork();
	const NANDRAD::HydraulicNetworkComponent *component = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(component != nullptr);

	m_ui->lineEditId->setText(tr("%1").arg(component->m_id));
	m_ui->lineEditName->setText(QString::fromStdString(component->m_displayName));
	m_ui->comboBoxComponentType->setCurrentText(m_mapComponents.key(component->m_modelType));

	item->setText(QString::fromStdString(component->m_displayName));

	updateTableView();
}

void SVDialogHydraulicComponents::updateTableView()
{
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	const VICUS::Network * network = currentNetwork();
	const NANDRAD::HydraulicNetworkComponent *tmpComp = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(tmpComp != nullptr);
	NANDRAD::HydraulicNetworkComponent component = *tmpComp;

	m_componentParModel = new HydraulicComponentParameterModel(this);
	connect(m_componentParModel, SIGNAL(editCompleted()), this, SLOT(on_componentParModel_editCompleted()));
	m_componentParModel->setComponent(component);
	m_ui->tableViewComponentParams->setModel(m_componentParModel);
	m_ui->tableViewComponentParams->show();
}


void SVDialogHydraulicComponents::modifyComponent()
{
	const VICUS::Network * network = currentNetwork();
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	const NANDRAD::HydraulicNetworkComponent *tmpComp = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(tmpComp != nullptr);
	NANDRAD::HydraulicNetworkComponent component = *tmpComp;

	component.m_displayName = m_ui->lineEditName->text().toStdString();
	component.m_modelType = NANDRAD::HydraulicNetworkComponent::modelType_t(m_mapComponents.value(m_ui->comboBoxComponentType->currentText()));

	if (component.m_modelType == NANDRAD::HydraulicNetworkComponent::NUM_MT)
		return;

	SVUndoNetworkHydraulicComponent *undo = new SVUndoNetworkHydraulicComponent("modified comp", network->m_id, component);
	undo->push();

	updateComponent();
}


void SVDialogHydraulicComponents::modifyTableView()
{
	HydraulicComponentItem *item = dynamic_cast<HydraulicComponentItem* >(m_ui->listWidget->currentItem());
	const VICUS::Network * network = currentNetwork();
	const NANDRAD::HydraulicNetworkComponent *tmpComp = VICUS::Project::element(network->m_hydraulicComponents, item->id());
	IBK_ASSERT(tmpComp != nullptr);
	NANDRAD::HydraulicNetworkComponent component = *tmpComp;


	// hier

	m_componentParModel->getComponentParameter(component.m_para);

	SVUndoNetworkHydraulicComponent *undo = new SVUndoNetworkHydraulicComponent("modified comp", network->m_id, component);
	undo->push();
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
	const VICUS::Project &p = project();
	const VICUS::Network * network = currentNetwork();
	unsigned int id = p.uniqueId(network->m_hydraulicComponents);
	QString name = "unnamed";

	// new list widget item
	HydraulicComponentItem * item = new HydraulicComponentItem(name);
	item->setName(name);
	item->setId(id);
	m_ui->listWidget->addItem(item);
	m_ui->listWidget->setCurrentItem(item);

	// new component
	NANDRAD::HydraulicNetworkComponent component;
	component.m_displayName = name.toStdString();
	component.m_id = id;
	component.m_modelType = NANDRAD::HydraulicNetworkComponent::NUM_MT;
	component.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::NUM_HT;

	SVUndoNetworkHydraulicComponent *undo = new SVUndoNetworkHydraulicComponent("modified comp", network->m_id, component);
	undo->push();

	updateComponent();
}

void SVDialogHydraulicComponents::on_toolButtonRemove_clicked()
{

}

void SVDialogHydraulicComponents::on_comboBoxNetwork_activated(int index)
{

}

void SVDialogHydraulicComponents::on_listWidget_itemClicked(QListWidgetItem *item)
{
	updateComponent();
}






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

QString HydraulicComponentItem::name() const
{
	return m_name;
}

void HydraulicComponentItem::setName(const QString &name)
{
	m_name = name;
}




// *** ComponentParameterModel ***

HydraulicComponentParameterModel::HydraulicComponentParameterModel(QObject *parent) : QAbstractTableModel(parent)
{
}

void HydraulicComponentParameterModel::setComponent(const NANDRAD::HydraulicNetworkComponent & component)
{
	m_component = component;
	m_parameterList = NANDRAD::HydraulicNetworkComponent::requiredParameter(m_component.m_modelType);
}

void HydraulicComponentParameterModel::getComponentParameter(IBK::Parameter m_para[])
{
	m_para = m_component.m_para;
}

int HydraulicComponentParameterModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_parameterList.size();
}

int HydraulicComponentParameterModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 3;
}

QVariant HydraulicComponentParameterModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	if (index.column()==0)
		return NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::para_t", m_parameterList[index.row()]);
	else if (index.column()==1)
		return m_component.m_para[m_parameterList[index.row()]].value;
	else if (index.column()==2)
		return NANDRAD::KeywordList::Unit("HydraulicNetworkComponent::para_t", m_parameterList[index.row()]);
}

QVariant HydraulicComponentParameterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		if (section == 0)
			return QString("Parameter");
		else if (section == 1)
			return QString("Value");
		else if (section == 2)
			return QString("Unit");
	}
	return QVariant();
}

bool HydraulicComponentParameterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

	if (index.column()==1){
		bool ok = false;
		double number = QtExt::Locale().toDouble(value.toString(), &ok);
		if (!ok)
			return false;
		m_component.m_para[m_parameterList[index.row()]].value = number;
		emit editCompleted();
	}
	return true;
}

Qt::ItemFlags HydraulicComponentParameterModel::flags(const QModelIndex &index) const
{
	if (index.column()==1)
		return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


