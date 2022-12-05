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

#include "SVDBEpdTableModel.h"

#include <map>
#include <algorithm>

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <VICUS_Construction.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <SVConversions.h>
#include <SVProjectHandler.h>
#include "SVConstants.h"
#include "SVStyle.h"

SVDBEpdTableModel::SVDBEpdTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBEpdTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::EpdDataset> & conDB = m_db->m_epdDatasets;

	int row = index.row();
	if (row >= (int)conDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::EpdDataset>::const_iterator it = conDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
				case ColModule				: {
					std::string keyword = VICUS::KeywordList::Keyword("EpdDataset::Module", it->second.m_module);
					return QString::fromStdString(keyword);
				}

				case ColType				: {
					std::string keyword = VICUS::KeywordList::Keyword("EpdDataset::Type", it->second.m_type);
					return QString::fromStdString(keyword);
				}
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (true /*it->second.isValid(m_db->m_materials)*/)
					return QIcon(":/gfx/actions/16x16/ok.png");
				else
					return QIcon(":/gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::TextAlignmentRole :
			switch (index.column()) {
				//				case ColUValue				: return int(Qt::AlignRight | Qt::AlignVCenter);
				//				case ColNumLayers			: return int(Qt::AlignRight | Qt::AlignVCenter);
			}
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


int SVDBEpdTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_epdDatasets.size();
}


QVariant SVDBEpdTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				case ColType				: return tr("Type");
				case ColModule				: return tr("Module");

				default: ;
			}
		} break;
			//		case Qt::FontRole : {
			//			QFont f;
			//			f.setBold(true);
			//			f.setPointSizeF(f.pointSizeF()*0.8);
			//			return f;
			//		}
	} // switch
	return QVariant();
}


void SVDBEpdTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}

QModelIndex SVDBEpdTableModel::addNewItem() {
	VICUS::EpdDataset epd;
	epd.m_displayName.setEncodedString("en:<new epd dataset>");
	epd.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_epdDatasets.add(epd);
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBEpdTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::EpdDataset> & db = m_db->m_epdDatasets;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::EpdDataset>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::EpdDataset newItem(it->second);
	unsigned int id = m_db->m_epdDatasets.add(newItem);
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBEpdTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_epdDatasets.remove(id);
	endRemoveRows();
}



void SVDBEpdTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBEpdTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBEpdTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBEpdTableModel::ColName, QHeaderView::Stretch);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBEpdTableModel::ColType, QHeaderView::ResizeToContents);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBEpdTableModel::ColModule, QHeaderView::ResizeToContents);
}


void SVDBEpdTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_epdDatasets[id]->m_local = local;
	m_db->m_epdDatasets.m_modified = true;
	setItemModified(id);
}

template <typename T>
void importDBElement(T & e, VICUS::Database<T> & db, std::map<unsigned int, unsigned int> & idSubstitutionMap,
					 const char * const importMsg, const char * const existingMsg)
{
	FUNCID(SVProjectHandler-importDBElement);
	// check, if element exists in built-in DB
	const T * existingElement = db.findEqual(e);
	if (existingElement == nullptr) {
		// element does not yet exist, import element; we try to keep the id from the embedded element
		// but if this is already taken, the database assigns a new unused id for use
		unsigned int oldId = e.m_id;
		unsigned int newId = db.add(e, oldId); // e.m_id gets modified here!

		VICUS::EpdDataset *epd = db[newId];
		if(epd != nullptr)
			epd->m_local = false;

//		IBK::IBK_Message( IBK::FormatString(importMsg)
//			.arg(e.m_displayName.string(),50,std::ios_base::left).arg(oldId).arg(newId),
//						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		if (newId != oldId)
			idSubstitutionMap[oldId] = newId;
	}
	else {
		// check if IDs match
		if (existingElement->m_id != e.m_id) {
			// we need to adjust the ID name of material
			idSubstitutionMap[e.m_id] = existingElement->m_id;
			IBK::IBK_Message( IBK::FormatString(existingMsg)
				.arg(e.m_displayName.string(),50,std::ios_base::left).arg(e.m_id).arg(existingElement->m_id),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}
}

void SVDBEpdTableModel::importDatasets(const std::vector<VICUS::EpdDataset> & epds){
	beginResetModel();
	// materials
	std::map<unsigned int, unsigned int> epdElementsIDMap; // newID = materialIDMap[oldID];
	for(unsigned int i=0; i<epds.size(); ++i) {
		VICUS::EpdDataset &epd = const_cast<VICUS::EpdDataset &>(epds[i]);

		importDBElement(epd, m_db->m_epdDatasets, epdElementsIDMap,
			"EPD-Element '%1' with #%2 imported -> new ID #%3.\n",
			"EPD-Element '%1' with #%2 exists already -> ID #%3.\n"
		);
	}

	endResetModel();
}


void SVDBEpdTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBEpdTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

