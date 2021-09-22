#include "FMUVariableTableModel.h"

#include "NandradFMUGeneratorWidget.h"

#include <QColor>

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
	if (!index.isValid())
		return QVariant();
	const NANDRAD::FMIVariableDefinition & var = (*m_availableVariables)[(size_t)index.row()];
	switch (role) {
		case Qt::DisplayRole :
			switch (index.column()) {
				case 0 : // name
					return QString::fromStdString(var.m_varName);
				case 1 : // object ID
					return var.m_objectId;
				case 2 : // vector value ID
					if (var.m_vectorIndex == NANDRAD::INVALID_ID)
						return QVariant();
					return var.m_vectorIndex;
				case 3 : // unit
					return QString::fromStdString(var.m_unit);
				case 4 : // FMI Variable Name
					return QString::fromStdString(var.m_fmiVarName);
				case 5 : // FMI value ref
					if (var.m_fmiValueRef == NANDRAD::INVALID_ID)
						return "---";
					return var.m_fmiValueRef;
				case 6 : // FMI type
					return "Real"; // for now always Real, QString::fromStdString(var.m_fmiTypeName);
				case 7 : // Description
					return QString::fromStdString(var.m_fmiVarDescription);
			}
		break;

		case Qt::EditRole :
			Q_ASSERT(index.column() == 4);
			return QString::fromStdString(var.m_fmiVarName);

		case Qt::FontRole :
			// vars with INVALID valueRef -> grey italic
			//      with valid value -> black, bold
			if (var.m_fmiValueRef == NANDRAD::INVALID_ID) {
				QFont f(m_itemFont);
				f.setItalic(true);
				return f;
			}
			else {
				QFont f(m_itemFont);
				f.setBold(true);
				return f;
			}

		case Qt::ForegroundRole :
			// vars with INVALID valueRef -> grey italic
			if (var.m_fmiValueRef == NANDRAD::INVALID_ID)
				return QColor(Qt::gray);
		break;

		// UserRole returns value reference
		case Qt::UserRole :
			return var.m_fmiValueRef;
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


Qt::ItemFlags FMUVariableTableModel::flags(const QModelIndex & index) const {
	if (index.column() == 4)
		// only allow name editing when variable is already configured
		if (index.data(Qt::UserRole).toUInt() != NANDRAD::INVALID_ID)
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	return QAbstractTableModel::flags(index); // call base class implementation
}


bool FMUVariableTableModel::setData(const QModelIndex & index, const QVariant & value, int /*role*/) {
	Q_ASSERT(index.isValid());
	Q_ASSERT(index.column() == 4);
	// error handling
	QString fmiVarName = value.toString().trimmed();
	// variable name must not be empty or only consist of white spaces; this is silently handled as error (because obvious)
	if (fmiVarName.isEmpty())
		return false;

	// ask Widged to rename and check variable
	NandradFMUGeneratorWidget *parentWidged = dynamic_cast<NandradFMUGeneratorWidget *>(parent());
	Q_ASSERT(parentWidged != nullptr);

	if (m_inputVariableTable) {
		// error handling is performed in 'NandradFMUGeneratorWidget'
		if (!parentWidged->renameInputVariable((size_t)index.row(), fmiVarName) )
			return false;
	}
	else {
		// error handling is performed in 'NandradFMUGeneratorWidget'
		if (!parentWidged->renameOutputVariable((size_t)index.row(), fmiVarName) )
			return false;
	}

	emit dataChanged(index, index);
	return true;
}


void FMUVariableTableModel::reset() {
	beginResetModel();
	endResetModel();
}


void FMUVariableTableModel::variableModified(unsigned int row) {
	// get index range
	QModelIndex left = index((int)row, 0);
	QModelIndex right = index((int)row, 7);
	emit dataChanged(left, right);
}
