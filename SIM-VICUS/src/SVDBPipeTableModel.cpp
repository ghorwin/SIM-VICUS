#include "SVDBPipeTableModel.h"

#include "SVConstants.h"

#include <QIcon>
#include <QFont>

#include <VICUS_NetworkPipe.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <NANDRAD_KeywordList.h>


SVDBPipeTableModel::SVDBPipeTableModel(QObject *parent, SVDatabase &db) :
	QAbstractTableModel(parent),
	m_db(&db)
{
//	// must only be created from SVDBBoundaryConditionEditDialog
//	Q_ASSERT(dynamic_cast<SVDBBoundaryConditionEditDialog*>(parent) != nullptr);
//	Q_ASSERT(m_db != nullptr);
}

SVDBPipeTableModel::~SVDBPipeTableModel() {
}


int SVDBPipeTableModel::columnCount ( const QModelIndex & ) const {
	return NumColumns;
}


QVariant SVDBPipeTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::NetworkPipe> & pipes = m_db->m_pipes;

	int row = index.row();
	if (row >= static_cast<int>(pipes.size()))
		return QVariant();

	std::map<unsigned int, VICUS::NetworkPipe>::const_iterator it = pipes.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QString::fromStdString(it->second.m_displayName);
			}
		} break;

		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
					return QSize(22, 16);
			} // switch
			break;

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;
	}

	return QVariant();
}


int SVDBPipeTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_pipes.size();
}


QVariant SVDBPipeTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
			}
		} break;

		case Qt::FontRole : {
			QFont f;
			f.setBold(true);
			f.setPointSizeF(f.pointSizeF()*0.8);
			return f;
		}
	} // switch
	return QVariant();
}


QModelIndex SVDBPipeTableModel::addNewItem() {
	VICUS::NetworkPipe pipe;
	pipe.m_displayName = "<new pipe>";

	pipe.m_roughness = 7e-6;
	pipe.m_lambdaWall = 0.4;
	pipe.m_diameterOutside = 0;
	pipe.m_wallThickness = 0;

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_pipes.add( pipe );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBPipeTableModel::addNewItem(VICUS::NetworkPipe pipe) {
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_pipes.add( pipe );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


bool SVDBPipeTableModel::deleteItem(QModelIndex index) {
	if (!index.isValid())
		return false;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_pipes.remove(id);
	endRemoveRows();
	return true;
}


void SVDBPipeTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


void SVDBPipeTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBPipeTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}


