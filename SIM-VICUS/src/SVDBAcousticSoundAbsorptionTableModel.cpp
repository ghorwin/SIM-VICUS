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

#include "SVDBAcousticSoundAbsorptionTableModel.h"

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <VICUS_AcousticSoundAbsorption.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <NANDRAD_KeywordList.h>

#include <SVConversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBAcousticSoundAbsorptionTableModel::SVDBAcousticSoundAbsorptionTableModel(QObject *parent, SVDatabase &db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBAcousticSoundAbsorptionTableModel::data ( const QModelIndex & index, int role) const {

	if (!index.isValid())
		return QVariant();

	if (index.column() == ColColor && role == Role_Color) {
		return true;
	}

	// readability improvement
	const VICUS::Database<VICUS::AcousticSoundAbsorption> & bcDB = m_db->m_acousticSoundAbsorptions;

	int row = index.row();
	if (row >= (int)bcDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::AcousticSoundAbsorption>::const_iterator it = bcDB.begin();
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
				std::string errorMsg = "";
				if (it->second.isValid())
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
				if (!it->second.isValid())
					return QString::fromStdString(it->second.m_errorMsg);
			}
		}
	}

	return QVariant();
}


int SVDBAcousticSoundAbsorptionTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_acousticSoundAbsorptions.size();
}


QVariant SVDBAcousticSoundAbsorptionTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


void SVDBAcousticSoundAbsorptionTableModel::resetModel() {

	beginResetModel();
	endResetModel();
}


QModelIndex SVDBAcousticSoundAbsorptionTableModel::addNewItem() {

	VICUS::AcousticSoundAbsorption bc;
	bc.m_displayName.setString(tr("<new acoustic sound absorption>").toStdString(), IBK::MultiLanguageString::m_language);
	bc.m_color = SVStyle::randomColor();

	//set default parameters

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_acousticSoundAbsorptions.add( bc );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBAcousticSoundAbsorptionTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::AcousticSoundAbsorption> & db = m_db->m_acousticSoundAbsorptions;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::AcousticSoundAbsorption>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::AcousticSoundAbsorption newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_acousticSoundAbsorptions.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBAcousticSoundAbsorptionTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_acousticSoundAbsorptions.remove(id);
	endRemoveRows();
}


void SVDBAcousticSoundAbsorptionTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticSoundAbsorptionTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticSoundAbsorptionTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticSoundAbsorptionTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticSoundAbsorptionTableModel::ColName, QHeaderView::Stretch);
}


void SVDBAcousticSoundAbsorptionTableModel::setItemLocal(const QModelIndex &index, bool local) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_acousticSoundAbsorptions[id]->m_local = local;
	m_db->m_acousticSoundAbsorptions.m_modified = true;
	setItemModified(id);
}


void SVDBAcousticSoundAbsorptionTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBAcousticSoundAbsorptionTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
