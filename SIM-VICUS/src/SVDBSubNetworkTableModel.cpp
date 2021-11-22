#include "SVDBSubNetworkTableModel.h"

#include "SVConstants.h"
#include "SVStyle.h"

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <QtExt_Conversions.h>

#include <VICUS_SubNetwork.h>



SVDBSubNetworkTableModel::SVDBSubNetworkTableModel(QObject *parent, SVDatabase &db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBSubNetworkTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.column() == ColColor && role == Role_Color) {
		return true;
	}

	// readability improvement
	const VICUS::Database<VICUS::SubNetwork> & bcDB = m_db->m_subNetworks;

	int row = index.row();
	if (row >= (int)bcDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::SubNetwork>::const_iterator it = bcDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
			}
		} break;

		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
				case ColColor :
					return QSize(22, 16);
			} // switch
		break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid(m_db->m_networkComponents, m_db->m_networkControllers, m_db->m_schedules))
					return QIcon("://gfx/actions/16x16/ok.png");
				else
					return QIcon("://gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::BackgroundRole : {
			if (index.column() == ColColor) {
				return it->second.m_color;
			}
		} break;

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;

		case Role_Local :
			return it->second.m_local;

		case Role_Referenced:
			return it->second.m_isReferenced;
	}

	return QVariant();
}


int SVDBSubNetworkTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_subNetworks.size();
}


QVariant SVDBSubNetworkTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


void SVDBSubNetworkTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBSubNetworkTableModel::addNewItem() {
	VICUS::SubNetwork bc;
	bc.m_displayName.setEncodedString("en:<new sub network>");
	bc.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_subNetworks.add( bc );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBSubNetworkTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::SubNetwork> & db = m_db->m_subNetworks;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::SubNetwork>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::SubNetwork newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_subNetworks.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBSubNetworkTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_subNetworks.remove(id);
	endRemoveRows();
}


void SVDBSubNetworkTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBSubNetworkTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBSubNetworkTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBSubNetworkTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBSubNetworkTableModel::ColName, QHeaderView::Stretch);
}

void SVDBSubNetworkTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_subNetworks[id]->m_local = local;
	m_db->m_subNetworks.m_modified = true;
	setItemModified(id);
}


void SVDBSubNetworkTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBSubNetworkTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
