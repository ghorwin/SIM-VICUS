#include "SVSimulationOutputTableModel.h"

SVSimulationOutputTableModel::SVSimulationOutputTableModel(QObject *parent) :
	QAbstractTableModel(parent)
{}


int SVSimulationOutputTableModel::rowCount(const QModelIndex & /*parent*/) const {
	int size = (int)m_outputDefinitions->size();
	return size;
}


int SVSimulationOutputTableModel::columnCount(const QModelIndex & /*parent*/) const {
	return 5;
}

QVariant SVSimulationOutputTableModel::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();
	const OutputDefinition & var = (*m_outputDefinitions)[(size_t)index.row()];
	switch (role) {
		case Qt::DisplayRole :
			switch (index.column()) {
				case 0 : // name
					return var.m_name;
				case 1 : // unit
					return QString::fromStdString(var.m_unit.name());
				case 2 : // description
					return var.m_description;
				case 3 : { // source object ids
					if(var.m_sourceObjectIds.empty())
						return QVariant();
					QString idString;
					for (unsigned int i=0; i<var.m_sourceObjectIds.size()-1; ++i)
						idString += QString::number(var.m_sourceObjectIds[i].first) + ",";
					idString += QString::number(var.m_sourceObjectIds[var.m_sourceObjectIds.size()-1].first);
					return idString;
				} break;
				case 4 : { // vectorIds
					if(var.m_vectorIds.empty())
						return QVariant();
					QString idString;
					for (unsigned int i=0; i<var.m_vectorIds.size()-1; ++i)
						idString += QString::number(var.m_vectorIds[i]) + ",";
					idString += QString::number(var.m_vectorIds[var.m_vectorIds.size()-1]);
					return idString;
				} break;
			}
		break;

		case Qt::FontRole :
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

		case Qt::ForegroundRole :
			// vars with INVALID valueRef -> grey italic
			if (!var.m_isActive)
				return QColor(Qt::gray);
		break;
	}
	return QVariant();
}


QVariant SVSimulationOutputTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	static QStringList headers = QStringList()
		<< tr("Output Name")
		<< tr("Unit")
		<< tr("Description")
		<< tr("Source object id(s)")
		<< tr("Vector indexes/ids");

	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole :
			return headers[section];
	}
	return QVariant();
}


Qt::ItemFlags SVSimulationOutputTableModel::flags(const QModelIndex & index) const {
	if (index.column() == 4)
		// only allow name editing when variable is already configured
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	return QAbstractTableModel::flags(index); // call base class implementation
}


bool SVSimulationOutputTableModel::setData(const QModelIndex & index, const QVariant & value, int /*role*/) {
	Q_ASSERT(index.isValid());
	Q_ASSERT(index.column() == 4); // only on column 3 data can be set
	// error handling
	QString fmiVarName = value.toString().trimmed();
	// variable name must not be empty or only consist of white spaces; this is silently handled as error (because obvious)
	if (fmiVarName.isEmpty())
		return false;

	emit dataChanged(index, index);
	return true;
}

void SVSimulationOutputTableModel::reset() {
	beginResetModel();
	endResetModel();
}

void SVSimulationOutputTableModel::updateOutputData(unsigned int row) {
	// get index range
	QModelIndex left = index((int)row, 0);
	QModelIndex right = index((int)row, 4);
	emit dataChanged(left, right);
}
