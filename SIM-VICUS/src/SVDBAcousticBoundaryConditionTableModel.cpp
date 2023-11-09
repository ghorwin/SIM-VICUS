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

#include "SVDBAcousticBoundaryConditionTableModel.h"

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <VICUS_AcousticBoundaryCondition.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <NANDRAD_KeywordList.h>

#include <SVConversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBAcousticBoundaryConditionTableModel::SVDBAcousticBoundaryConditionTableModel(QObject *parent, SVDatabase &db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBAcousticBoundaryConditionTableModel::data ( const QModelIndex & index, int role) const {

	if (!index.isValid())
		return QVariant();

	if (index.column() == ColColor && role == Role_Color) {
		return true;
	}

	// readability improvement
	const VICUS::Database<VICUS::AcousticBoundaryCondition> & bcDB = m_db->m_acousticBoundaryConditions;

	int row = index.row();
	if (row >= (int)bcDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::AcousticBoundaryCondition>::const_iterator it = bcDB.begin();
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
				if (it->second.isValid(m_db->m_acousticSoundAbsorptions))
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
				if (!it->second.isValid(m_db->m_acousticSoundAbsorptions))
					return QString::fromStdString(it->second.m_errorMsg);
			}
		}
	}

	return QVariant();
}


int SVDBAcousticBoundaryConditionTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_acousticBoundaryConditions.size();
}


QVariant SVDBAcousticBoundaryConditionTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


void SVDBAcousticBoundaryConditionTableModel::resetModel() {

	beginResetModel();
	endResetModel();
}


QModelIndex SVDBAcousticBoundaryConditionTableModel::addNewItem() {

	VICUS::AcousticBoundaryCondition bc;
	bc.m_displayName.setString(tr("<new acoustic boundary condition>").toStdString(), IBK::MultiLanguageString::m_language);
	bc.m_color = SVStyle::randomColor();


	// ensures there is atleast one item int the Acoustic Sound Absorption database
	if (m_db->m_acousticSoundAbsorptions.empty()){
			VICUS::AcousticSoundAbsorption ac;
			ac.m_color = SVStyle::randomColor();
			ac.m_displayName.setString(tr("<new acoustic sound absorption>").toStdString(), IBK::MultiLanguageString::m_language);
			m_db->m_acousticSoundAbsorptions.add(ac);
	}

	unsigned int defaultSoundAbsorptionId = 0;
	if (!m_db->m_acousticSoundAbsorptions.empty())
			defaultSoundAbsorptionId = m_db->m_acousticSoundAbsorptions.begin()->first;
        VICUS::AcousticSoundAbsorptionPartition layer(0.1, defaultSoundAbsorptionId);
        bc.m_soundAbsorptionPartition.push_back(layer);
	//set default parameters

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_acousticBoundaryConditions.add( bc );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBAcousticBoundaryConditionTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::AcousticBoundaryCondition> & db = m_db->m_acousticBoundaryConditions;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::AcousticBoundaryCondition>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::AcousticBoundaryCondition newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_acousticBoundaryConditions.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBAcousticBoundaryConditionTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_acousticBoundaryConditions.remove(id);
	endRemoveRows();
}


void SVDBAcousticBoundaryConditionTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticBoundaryConditionTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticBoundaryConditionTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticBoundaryConditionTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBAcousticBoundaryConditionTableModel::ColName, QHeaderView::Stretch);
}


void SVDBAcousticBoundaryConditionTableModel::setItemLocal(const QModelIndex &index, bool local) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_acousticBoundaryConditions[id]->m_local = local;
	m_db->m_acousticBoundaryConditions.m_modified = true;
	setItemModified(id);
}


void SVDBAcousticBoundaryConditionTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBAcousticBoundaryConditionTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
