#include "SVDBSubNetworkTableModel.h"

#include "SVConstants.h"

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <QtExt_LanguageHandler.h>

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

	// readability improvement
	const VICUS::Database<VICUS::SubNetwork> & db = m_db->m_subNetworks;

	int row = index.row();
	if (row >= (int)db.size())
		return QVariant();

	std::map<unsigned int, VICUS::SubNetwork>::const_iterator it = db.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			// Note: when accessing multilanguage strings below, take name in current language or if missing, "all"
			std::string langId = QtExt::LanguageHandler::instance().langId().toStdString();
			std::string fallBackLangId = "en";

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QString::fromStdString(it->second.m_displayName.string(langId, fallBackLangId));
			}
		} break;

		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
					return QSize(22, 16);
			} // switch
		break;

		case Qt::TextAlignmentRole :
			switch (index.column()) {
				case ColId					: return int(Qt::AlignLeft | Qt::AlignVCenter);
				case ColName				: return int(Qt::AlignLeft | Qt::AlignVCenter);
				case ColCheck				: return int(Qt::AlignCenter | Qt::AlignVCenter);
			}
			break;

		case Qt::DecorationRole : {
			switch (index.column()) {
				case ColCheck:
					if (it->second.isValid())
						return QIcon("://gfx/actions/16x16/ok.png");
					else
						return QIcon("://gfx/actions/16x16/error.png");

			} // switch
		} break;

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;
	}

	return QVariant();
}


int SVDBSubNetworkTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_schedules.size();
}


QVariant SVDBSubNetworkTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				case ColCheck				: break;
			}
		} break;

		case Qt::ToolTipRole:{
			switch (section) {
				case ColCheck				: return QString(tr("Indicates if schedule data is valid."));
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
	VICUS::SubNetwork subNet;
	subNet.m_displayName.setEncodedString("en:<new schedule>");

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_subNetworks.add( subNet );
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
	tableView->horizontalHeader()->setSectionResizeMode(SVDBSubNetworkTableModel::ColName, QHeaderView::Stretch);
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
