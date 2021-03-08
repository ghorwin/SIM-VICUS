#include "SVDBZoneTemplateTreeModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_ZoneTemplate.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBZoneTemplateTreeModel::SVDBZoneTemplateTreeModel(QObject * parent, SVDatabase & db) :
	QAbstractItemModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBZoneTemplateTreeModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;

	// index is a top-level item
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it;
	if (index.internalPointer() == nullptr) {

		int row = index.row();
		if (row >= (int)db.size())
			return QVariant();

		it = db.begin();
		std::advance(it, row);

		switch (role) {
			case Role_Id :
				return it->first;

			case Qt::DisplayRole : {
				switch (index.column()) {
					case ColId					: return it->first;
					case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
				}
			} break;

			case Qt::DecorationRole : {
				if (index.column() == ColCheck) {
					if (it->second.isValid())
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
		}
	}
	else {
		// sub-template

		QModelIndex templateIndex = index.parent();

		int row = templateIndex.row();
		if (row >= (int)db.size())
			return QVariant();

		it = db.begin();
		std::advance(it, row);

		const VICUS::ZoneTemplate & zt = it->second;

		int subTemplateIndex = index.row();
		VICUS::ZoneTemplate::SubTemplateType subType = zt.usedReference((unsigned int)subTemplateIndex);

		// subtype role
		if (role == Qt::UserRole + 20) {
			return subType;
		}

		// ID role
		if (role == Role_Id ||
			(role == Qt::DisplayRole && index.column() == ColId))
		{
			return zt.m_idReferences[subType];
		}

		// different handling based on referenced sub-type
		switch (subType) {
			case VICUS::ZoneTemplate::ST_IntLoadPerson:  {
				// lookup item in question
				const VICUS::InternalLoad * iload = m_db->m_internalLoads[zt.m_idReferences[subType]];
				// Mind: il might be a nullptr, if index wasn't given
				if (role == Qt::DisplayRole && index.column() == ColName) {
					if (iload == nullptr)
						return tr("<invalid ID reference>");
					else
						return QtExt::MultiLangString2QString(iload->m_displayName);
				}
				else if (role == Qt::BackgroundRole && index.column() == ColColor) {
					if (iload == nullptr) return QVariant();
					else return iload->m_color;
				}
			} break;
			case VICUS::ZoneTemplate::ST_IntLoadEquipment:
			case VICUS::ZoneTemplate::ST_IntLoadLighting:
			case VICUS::ZoneTemplate::ST_IntLoadOther:
			case VICUS::ZoneTemplate::ST_ControlThermostat:
			case VICUS::ZoneTemplate::NUM_ST:
			break;
		}

	}

	// common handling for templates and sub-templates
	switch (role) {

		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
				case ColColor :
					return QSize(22, 16);
			} // switch
			break;

		case Role_BuiltIn :
			return it->second.m_builtIn;
	}

	return QVariant();
}


int SVDBZoneTemplateTreeModel::rowCount ( const QModelIndex & parent) const {
	// top-level - number of zone templates
	if (!parent.isValid())
		return (int)m_db->m_zoneTemplates.size();
	// sub-template nodes to not have children themselves
	if (parent.internalPointer() != nullptr)
		return 0;
	// lookup currently selected zone template
	int row = parent.row();
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT ((unsigned int)row < db.size());

	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, row);

	// return number of assigned sub-templates
	return (int)it->second.subTemplateCount();
}


QVariant SVDBZoneTemplateTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
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


QModelIndex SVDBZoneTemplateTreeModel::index(int row, int column, const QModelIndex & parent) const {
	if (!parent.isValid()) {
		return createIndex(row, column, nullptr); // top-level items have a nullptr for identification
	}
	// child item,
	int parentRow = parent.row();
	// take pointer to item
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, parentRow);
	// child items have a pointer to the zone template they belong to as identification
	return createIndex(row, column, (void*)(&it->second));
}


QModelIndex SVDBZoneTemplateTreeModel::parent(const QModelIndex & child) const {
	// nullptr means top-level it
	if (child.internalPointer() == nullptr)
		return QModelIndex();
	else {
		// get internal pointer and lookup item by id
		const VICUS::ZoneTemplate * ptr = reinterpret_cast<const VICUS::ZoneTemplate *>(child.internalPointer());
		// search DB and get the row index of this item
		std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = m_db->m_zoneTemplates.begin();
		int i=0;
		for (; ptr != &it->second && (unsigned int)i<m_db->m_zoneTemplates.size(); ++i, ++it);
		Q_ASSERT((unsigned int)i != m_db->m_zoneTemplates.size());
		return createIndex(i, 0, nullptr);
	}
}


void SVDBZoneTemplateTreeModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneTemplateTreeModel::addNewItem() {
	VICUS::ZoneTemplate c;
	c.m_displayName.setEncodedString("en:<new zone template>");
	c.m_color = SVStyle::randomColor();
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_zoneTemplates.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneTemplateTreeModel::addChildItem(const QModelIndex & templateIndex, int subTemplateType, unsigned int subTemplateID) {
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT(templateIndex.isValid() && templateIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, templateIndex.row());
	// now determine which row needs to be inserted
	int rowIndex = 0;
	for (int i=0; subTemplateType != i && i<VICUS::ZoneTemplate::NUM_ST; ++i) {
		if (it->second.m_idReferences[i] != VICUS::INVALID_ID)
			++rowIndex;
	}
	beginInsertRows(templateIndex, rowIndex, rowIndex);
	VICUS::ZoneTemplate * zt = const_cast<VICUS::ZoneTemplate*>(m_db->m_zoneTemplates[it->second.m_id]);
	Q_ASSERT(zt != nullptr);
	zt->m_idReferences[subTemplateType] = subTemplateID;
	endInsertRows();
	m_db->m_zoneTemplates.m_modified = true;
	return index(rowIndex, 0, templateIndex);
}


QModelIndex SVDBZoneTemplateTreeModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::ZoneTemplate newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_zoneTemplates.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneTemplateTreeModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_zoneTemplates.remove(id);
	endRemoveRows();
}


void SVDBZoneTemplateTreeModel::deleteChildItem(const QModelIndex & templateIndex, int subTemplateType) {
	if (!templateIndex.isValid())
		return;
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT(templateIndex.isValid() && templateIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, templateIndex.row());
	// now determine which row needs to be removed
	int rowIndex = 0;
	for (int i=0; subTemplateType != i && i<VICUS::ZoneTemplate::NUM_ST; ++i) {
		if (it->second.m_idReferences[i] != VICUS::INVALID_ID)
			++rowIndex;
	}
	beginRemoveRows(templateIndex, rowIndex, rowIndex);
	VICUS::ZoneTemplate * zt = const_cast<VICUS::ZoneTemplate*>(m_db->m_zoneTemplates[it->second.m_id]);
	Q_ASSERT(zt != nullptr);
	zt->m_idReferences[subTemplateType] = VICUS::INVALID_ID;
	endRemoveRows();
}


void SVDBZoneTemplateTreeModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0, QModelIndex());
	int rows = rowCount(left);
	QModelIndex right = index(rows-1, NumColumns-1, left);
	emit dataChanged(left, right);
}


QModelIndex SVDBZoneTemplateTreeModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0, QModelIndex());
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

