#include "InputVariablesTableModel.h"

InputVariablesTableModel::InputVariablesTableModel(QObject *parent)
	: QAbstractTableModel(parent)
{
}

QVariant InputVariablesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	// FIXME: Implement me!
}

int InputVariablesTableModel::rowCount(const QModelIndex &parent) const
{
	return m_availableInputVariables.size();
}

int InputVariablesTableModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return 8;
}

QVariant InputVariablesTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	// FIXME: Implement me!
	return QVariant();
}
