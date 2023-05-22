#include "SVDBNetworkControllerTableModel.h"
#include "SVConstants.h"

#include <SVConversions.h>

#include <QIcon>
#include <QHeaderView>
#include <QTableView>

SVDBNetworkControllerTableModel::SVDBNetworkControllerTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBNetworkControllerTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::NetworkController> & ctrDB = m_db->m_networkControllers;

	int row = index.row();
	if (row >= (int)ctrDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::NetworkController>::const_iterator it = ctrDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
				case ColType				: return VICUS::KeywordList::Keyword("NetworkController::ControlledProperty",
																				 it->second.m_controlledProperty);
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid(m_db->m_schedules))
					return QIcon(":/gfx/actions/16x16/ok.png");
				else
					return QIcon(":/gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::BackgroundRole : {
			if (index.column() == ColColor) {
				return it->second.m_color;
			}
		} break;

		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
				case ColColor :
					return QSize(22, 16);
			} // switch
			break;

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;

		case Role_Local :
			return it->second.m_local;

		case Role_Referenced:
			return it->second.m_isReferenced;

		case Qt::ToolTipRole: {
			if(index.column() == ColCheck) {
				std::string errorMsg = "";
				if (!it->second.isValid(m_db->m_schedules))
					return QString::fromStdString(it->second.m_errorMsg);
			}
		}
	}

	return QVariant();
}


int SVDBNetworkControllerTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_networkControllers.size();
}


QVariant SVDBNetworkControllerTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				case ColType				: return tr("Property");
				default: ;
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


void SVDBNetworkControllerTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBNetworkControllerTableModel::addNewItem() {
	VICUS::NetworkController c;
	c.m_displayName.setString(tr("<new network controller>").toStdString(), IBK::MultiLanguageString::m_language);
	c.m_modelType = VICUS::NetworkController::MT_Constant;
	c.m_controlledProperty = VICUS::NetworkController::CP_TemperatureDifference;
	c.m_controllerType = VICUS::NetworkController::CT_PController;
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_networkControllers.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBNetworkControllerTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::NetworkController> & db = m_db->m_networkControllers;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::NetworkController>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::NetworkController newItem(it->second);
	unsigned int id = m_db->m_networkControllers.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBNetworkControllerTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_networkControllers.remove(id);
	endRemoveRows();
}


void SVDBNetworkControllerTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkControllerTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkControllerTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkControllerTableModel::ColName, QHeaderView::Stretch);
}


void SVDBNetworkControllerTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_networkControllers[id]->m_local = local;
	m_db->m_networkControllers.m_modified = true;
	setItemModified(id);
}


void SVDBNetworkControllerTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBNetworkControllerTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
