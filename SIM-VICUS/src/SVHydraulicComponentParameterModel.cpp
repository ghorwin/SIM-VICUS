#include "SVHydraulicComponentParameterModel.h"

#include "NANDRAD_KeywordList.h"

#include "QtExt_Locale.h"


SVHydraulicComponentParameterModel::SVHydraulicComponentParameterModel(QObject *parent) : QAbstractTableModel(parent)
{
}

void SVHydraulicComponentParameterModel::setComponent(const NANDRAD::HydraulicNetworkComponent & component)
{
	m_component = component;
	m_parameterList = NANDRAD::HydraulicNetworkComponent::requiredParameter(m_component.m_modelType);
}

void SVHydraulicComponentParameterModel::getComponentParameter(IBK::Parameter m_para[])
{
	for (unsigned i = 0; i < NANDRAD::HydraulicNetworkComponent::NUM_P; ++i)
		*(m_para + i) = *(m_component.m_para + i);
}

int SVHydraulicComponentParameterModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_parameterList.size();
}

int SVHydraulicComponentParameterModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 3;
}

QVariant SVHydraulicComponentParameterModel::data(const QModelIndex &index, int role) const
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

QVariant SVHydraulicComponentParameterModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool SVHydraulicComponentParameterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

	if (index.column()==1){
		bool ok = false;
		double number = QtExt::Locale().toDouble(value.toString(), &ok);
		if (!ok)
			return false;
		NANDRAD::KeywordList::setParameter(m_component.m_para, "HydraulicNetworkComponent::para_t",
										   m_parameterList[index.row()], number);
		emit editCompleted();
	}
	return true;
}

Qt::ItemFlags SVHydraulicComponentParameterModel::flags(const QModelIndex &index) const
{
	if (index.column()==1)
		return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

