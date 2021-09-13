#include "FMUVariableTableModel.h"

FMUVariableTableModel::FMUVariableTableModel(QObject * parent, bool inputVariableTable) :
	QAbstractTableModel(parent),
	m_inputVariableTable(inputVariableTable)
{

}

int FMUVariableTableModel::rowCount(const QModelIndex & /*parent*/) const {
	return m_availableVariables->size();
}


int FMUVariableTableModel::columnCount(const QModelIndex & /*parent*/) const {
	return 8;
}


QVariant FMUVariableTableModel::data(const QModelIndex & index, int role) const {
	switch (role) {
		case Qt::DisplayRole :
			switch (index.column()) {
				case 1 : // object ID
					return (*m_availableVariables)[(size_t)index.row()].m_objectId;
			}
		break;

		case Qt::FontRole :
			// vars with INVALID valueRef -> grey italic
			//      with valid value -> black, bold
		break;

		case Qt::ForegroundRole :
			// vars with INVALID valueRef -> grey italic
		break;
	}
	return QVariant();
}


QVariant FMUVariableTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	static QStringList headers = QStringList()
		<< tr("NANDRAD Variable Name")
		<< tr("Object ID")
		<< tr("Vector value index/ID")
		<< tr("Unit")
		<< tr("FMI Variable Name")
		<< tr("FMI value reference")
		<< tr("FMI Type")
		<< tr("Description");

	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole :
			return headers[section];
	}
	return QVariant();
}


void FMUVariableTableModel::reset() {
	beginResetModel();
	endResetModel();
}
