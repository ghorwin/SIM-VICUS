#include "SVDBAcousticComponentTableModel.h"
#include <SVConversions.h>
#include "SVConstants.h"
#include "SVStyle.h"
#include <VICUS_KeywordListQt.h>
#include <QTableView>
#include <QHeaderView>

SVDBAcousticComponentTableModel::SVDBAcousticComponentTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBAcousticComponentTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.column() == ColColor && role == Role_Color) {
		return true;
	}

	// readability improvement
	const VICUS::Database<VICUS::AcousticComponent> & comDB = m_db->m_acousticComponents;

	int row = index.row();
	if (row >= (int)comDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::AcousticComponent>::const_iterator it = comDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
//				case ColType				: return VICUS::KeywordListQt::Description("Component::ComponentType", it->second.m_type);
			}
		} break;

		//TODO Anton: soll etwas hier geprüft werden
		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (true /*it->second.isValid(m_db->m_materials, m_db->m_constructions, m_db->m_boundaryConditions, m_db->m_schedules)*/)
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

		/*case Qt::ToolTipRole: {
			if(index.column() == ColCheck) {
				std::string errorMsg = "";
				if (!it->second.isValid(m_db->m_materials, m_db->m_constructions, m_db->m_boundaryConditions, m_db->m_schedules))
					return QString::fromStdString(it->second.m_errorMsg);
			}
		}*/
	}

	return QVariant();
}

int SVDBAcousticComponentTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_acousticComponents.size();
}

QVariant SVDBAcousticComponentTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
//				case ColType				: return tr("Type");
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

void SVDBAcousticComponentTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBAcousticComponentTableModel::addNewItem() {
	VICUS::AcousticComponent ac;
	ac.m_displayName.setEncodedString("en:<new acoustic component>");
	ac.m_color = SVStyle::randomColor();
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_acousticComponents.add( ac );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}

QModelIndex SVDBAcousticComponentTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::AcousticComponent> & db = m_db->m_acousticComponents;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::AcousticComponent>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::AcousticComponent newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_acousticComponents.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBAcousticComponentTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_acousticComponents.remove(id);
	endRemoveRows();
}



void SVDBAcousticComponentTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticComponentTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticComponentTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticComponentTableModel::ColName, QHeaderView::Stretch);
}

void SVDBAcousticComponentTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_acousticComponents[id]->m_local = local;
	m_db->m_acousticComponents.m_modified = true;
	setItemModified(id);
}


void SVDBAcousticComponentTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBAcousticComponentTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

