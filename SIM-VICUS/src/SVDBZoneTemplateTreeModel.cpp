#include "SVDBZoneTemplateTreeModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_ZoneTemplate.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"

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

	int row = index.row();
	if (row >= (int)db.size())
		return QVariant();

	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
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
	}

	return QVariant();
}


int SVDBZoneTemplateTreeModel::rowCount ( const QModelIndex & parent) const {
	// top-level - number of zone templates
	if (!parent.isValid())
		return (int)m_db->m_zoneTemplates.size();
	// lookup currently selected zone template
	int row = parent.row();
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT ((unsigned int)row < db.size());

	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, row);

	// return number of assigned sub-templates
	return it->second.subTemplateCount();
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
}


QModelIndex SVDBZoneTemplateTreeModel::parent(const QModelIndex & child) const {
}


void SVDBZoneTemplateTreeModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneTemplateTreeModel::addNewItem() {
	VICUS::Component c;
	c.m_displayName.setEncodedString("en:<new component type>");
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_components.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneTemplateTreeModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::Component> & db = m_db->m_components;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::Component>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::Component newItem(it->second);
	unsigned int id = m_db->m_components.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneTemplateTreeModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_constructions.remove(id);
	endRemoveRows();
}


void SVDBZoneTemplateTreeModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
//	QModelIndex left = index(idx.row(), 0);
//	QModelIndex right = index(idx.row(), NumColumns-1);
//	emit dataChanged(left, right);
}


QModelIndex SVDBZoneTemplateTreeModel::indexById(unsigned int id) const {
//	for (int i=0; i<rowCount(); ++i) {
//		QModelIndex idx = index(i, 0);
//		if (data(idx, Role_Id).toUInt() == id)
//			return idx;
//	}
	return QModelIndex();
}

