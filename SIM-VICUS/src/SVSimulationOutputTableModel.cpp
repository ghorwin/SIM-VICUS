#include "SVSimulationOutputTableModel.h"

#include<QIcon>

SVSimulationOutputTableModel::SVSimulationOutputTableModel(QObject *parent) :
	QAbstractTableModel(parent)
{}


int SVSimulationOutputTableModel::rowCount(const QModelIndex & /*parent*/) const {
	if (m_outputDefinitions == nullptr)
		return 0;

	return (int)m_outputDefinitions->size();
}


int SVSimulationOutputTableModel::columnCount(const QModelIndex & /*parent*/) const {
	return 5;
}

QVariant SVSimulationOutputTableModel::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();
	OutputDefinition & var = (*m_outputDefinitions)[(size_t)index.row()];
	switch (role) {
		case Qt::DisplayRole : {
			switch (index.column()) {
				case 1 : // type
					return QString::fromStdString(var.m_outputdefinition.m_type);
				case 2 : // name
					return QString::fromStdString(var.m_outputdefinition.m_name);
				case 3 : // unit
					return QString::fromStdString(var.m_outputdefinition.m_unit.name());
//				case 4 : // description
//					return var.m_description;
				case 4 : // source
					return (var.m_outputdefinition.m_sourceObjectIds.size() == 1) ?
							QString::fromStdString(var.m_outputdefinition.m_idToSourceObject[var.m_outputdefinition.m_sourceObjectIds[0]].m_displayName) : "*";
			}
		} break;

		case Qt::FontRole : {
			// vars with INVALID valueRef -> grey italic
			//      with valid value -> black, bold
			if (!var.m_isActive) {
				QFont f(m_itemFont);
				f.setItalic(true);
				return f;
			}
			else {
				QFont f(m_itemFont);
				f.setBold(true);
				return f;
			}
		} break;
		case Qt::ForegroundRole : {
			// vars with INVALID valueRef -> grey italic
			//	if (!var.m_isActive)
				// return QColor(Qt::gray);
		} break;
		case Qt::DecorationRole : {
			switch (index.column()) {
				case 0:
					if (var.m_isActive)
						return QIcon("://gfx/actions/16x16/ok.png");
			}
		} break;
		case Qt::UserRole : {
			switch (index.column()) {
				case 0:
					if (var.m_isActive)
						return 1;
					else
						return 0;
			}
		} break;
	}
	return QVariant();
}


QVariant SVSimulationOutputTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	static QStringList headers = QStringList()
		<< ""
		<< tr("Type")
		<< tr("Name")
		<< tr("Unit")
//		<< tr("Description")
		<< tr("Source");

	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole :
			return headers[section];

	}
	return QVariant();
}


Qt::ItemFlags SVSimulationOutputTableModel::flags(const QModelIndex & index) const {
	return QAbstractTableModel::flags(index); // call base class implementation
}


void SVSimulationOutputTableModel::reset() {
	beginResetModel();
	endResetModel();
}

void SVSimulationOutputTableModel::updateOutputData(unsigned int row) {
	// get index range
	QModelIndex left = index((int)row, 0);
	QModelIndex right = index((int)row, 3);
	emit dataChanged(left, right);
}
