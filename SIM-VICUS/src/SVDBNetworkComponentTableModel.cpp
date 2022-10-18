/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVDBNetworkComponentTableModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_NetworkComponent.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include "SVConversions.h"
#include "SVConstants.h"


SVDBNetworkComponentTableModel::SVDBNetworkComponentTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBNetworkComponentTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::NetworkComponent> & comDB = m_db->m_networkComponents;

	int row = index.row();
	if (row >= (int)comDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::NetworkComponent>::const_iterator it = comDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
					// Note: description is too long here for "Type"
				case ColType				: return VICUS::KeywordList::Keyword("NetworkComponent::ModelType",
																				 it->second.m_modelType);
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


int SVDBNetworkComponentTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_networkComponents.size();
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


void SVDBNetworkComponentTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBNetworkComponentTableModel::addNewItem() {
	VICUS::NetworkComponent c;
	c.m_displayName.setEncodedString("en:<new component type>");
	c.m_modelType = VICUS::NetworkComponent::MT_HeatExchanger;
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_networkComponents.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBNetworkComponentTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::NetworkComponent> & db = m_db->m_networkComponents;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::NetworkComponent>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::NetworkComponent newItem(it->second);
	unsigned int id = m_db->m_networkComponents.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBNetworkComponentTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_networkComponents.remove(id);
	endRemoveRows();
}


void SVDBNetworkComponentTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkComponentTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkComponentTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBNetworkComponentTableModel::ColName, QHeaderView::Stretch);
}


void SVDBNetworkComponentTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_networkComponents[id]->m_local = local;
	m_db->m_networkComponents.m_modified = true;
	setItemModified(id);
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

