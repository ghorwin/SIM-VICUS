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

#include "SVDBZoneIdealHeatingCoolingTableModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_KeywordListQt.h>

#include <SVConversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBZoneIdealHeatingCoolingTableModel::SVDBZoneIdealHeatingCoolingTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


int SVDBZoneIdealHeatingCoolingTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_zoneIdealHeatingCooling.size();
}


QVariant SVDBZoneIdealHeatingCoolingTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::ZoneIdealHeatingCooling> & ideal = m_db->m_zoneIdealHeatingCooling;

	int row = index.row();
	if (row >= (int)ideal.size())
		return QVariant();

	std::map<unsigned int, VICUS::ZoneIdealHeatingCooling>::const_iterator it = ideal.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
				case ColSource				: return QtExt::MultiLangString2QString(it->second.m_dataSource);
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid()) // for now just check for validity
					return QIcon(":/gfx/actions/16x16/ok.png");
				else
					return QIcon(":/gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::TextAlignmentRole :
			//if (index.column() >= ColRho && index.column() < NumColumns)
			//	return int(Qt::AlignRight | Qt::AlignVCenter);
			break;

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

		case Role_Local :
			return it->second.m_local;

		case Role_Referenced:
			return it->second.m_isReferenced;
	}

	return QVariant();
}


QVariant SVDBZoneIdealHeatingCoolingTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				//case ColCategory			: return tr("Category");
				case ColSource				: return tr("Source");
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


void SVDBZoneIdealHeatingCoolingTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneIdealHeatingCoolingTableModel::addNewItem() {
	VICUS::ZoneIdealHeatingCooling ideal;
	ideal.m_displayName.setString(tr("<new ideal heating cooling model>").toStdString(), IBK::MultiLanguageString::m_language);

	// set default parameters

	VICUS::KeywordList::setParameter(ideal.m_para, "ZoneIdealHeatingCooling::para_t", VICUS::ZoneIdealHeatingCooling::P_HeatingLimit, 50.0);
	VICUS::KeywordList::setParameter(ideal.m_para, "ZoneIdealHeatingCooling::para_t", VICUS::ZoneIdealHeatingCooling::P_CoolingLimit, 40.0);

	ideal.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_zoneIdealHeatingCooling.add( ideal );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneIdealHeatingCoolingTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::ZoneIdealHeatingCooling> & db = m_db->m_zoneIdealHeatingCooling;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneIdealHeatingCooling>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::ZoneIdealHeatingCooling newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_zoneIdealHeatingCooling.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneIdealHeatingCoolingTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_zoneIdealHeatingCooling.remove(id);
	endRemoveRows();
}


void SVDBZoneIdealHeatingCoolingTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneIdealHeatingCoolingTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneIdealHeatingCoolingTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneIdealHeatingCoolingTableModel::ColName, QHeaderView::Stretch);
}


void SVDBZoneIdealHeatingCoolingTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_zoneIdealHeatingCooling[id]->m_local = local;
	m_db->m_zoneIdealHeatingCooling.m_modified = true;
	setItemModified(id);
}


void SVDBZoneIdealHeatingCoolingTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBZoneIdealHeatingCoolingTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
