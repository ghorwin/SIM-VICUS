#include "SVDBNetworkComponentTableModel.h"

#include <VICUS_NetworkComponent.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"

SVDBNetworkComponentTableModel::SVDBNetworkComponentTableModel(QObject * parent, SVDatabase & db) :
	QAbstractTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}

SVDBNetworkComponentTableModel::~SVDBNetworkComponentTableModel() {
}

int SVDBNetworkComponentTableModel::columnCount ( const QModelIndex & ) const {
	return NumColumns;
}

QVariant SVDBNetworkComponentTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::Component> & comDB = m_db->m_components;

	int row = index.row();
	if (row >= (int)comDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::Component>::const_iterator it = comDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			// Note: when accessing multilanguage strings below, take name in current language or if missing, "all"
			std::string langId = QtExt::LanguageHandler::instance().langId().toStdString();
			std::string fallBackLangId = "en";

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QString::fromStdString(it->second.m_displayName.string(langId, fallBackLangId));
				case ColType				: return VICUS::KeywordListQt::Description("Component::ComponentType", it->second.m_type);
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid(m_db->m_materials, m_db->m_constructions, m_db->m_boundaryConditions))
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


int SVDBNetworkComponentTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_components.size();
}


QVariant SVDBNetworkComponentTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				case ColType				: return tr("Type");
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


QModelIndex SVDBNetworkComponentTableModel::addNewItem() {
	VICUS::NetworkComponent c;
	c.m_displayName.setEncodedString("en:<new component type>");
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_networkComponents.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBNetworkComponentTableModel::addNewItem(VICUS::Component c) {
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_components.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBNetworkComponentTableModel::findItem(unsigned int componentId) const {
	// select component with given matId
	for (int i=0, count = rowCount(); i<count; ++i) {
		QModelIndex sourceIndex = index(i,0);
		if (data(sourceIndex, Role_Id).toUInt() == componentId)
			return sourceIndex;
	}
	return QModelIndex();
}


bool SVDBNetworkComponentTableModel::deleteItem(QModelIndex index) {
	if (!index.isValid())
		return false;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_components.remove(id);
	endRemoveRows();
	return true;
}


void SVDBNetworkComponentTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


void SVDBNetworkComponentTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBNetworkComponentTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

