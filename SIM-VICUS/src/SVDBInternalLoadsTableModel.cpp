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

#include "SVDBInternalLoadsTableModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_KeywordListQt.h>

#include <SVConversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBInternalLoadsTableModel::SVDBInternalLoadsTableModel(QObject * parent, SVDatabase & db, VICUS::InternalLoad::Category t) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db),
	m_category(t)
{
	Q_ASSERT(m_db != nullptr);
}


int SVDBInternalLoadsTableModel::rowCount ( const QModelIndex & ) const {
	// count number of type-specific elements
	const VICUS::Database<VICUS::InternalLoad> & intLoadDB = m_db->m_internalLoads;
	unsigned int count=0;
	for (std::map<unsigned int, VICUS::InternalLoad>::const_iterator it = intLoadDB.begin();
		 it != intLoadDB.end(); ++it)
	{
		if(it->second.m_category == m_category)
			++count;
	}
	return (int)count;
}


QVariant SVDBInternalLoadsTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::InternalLoad> & intLoadDB = m_db->m_internalLoads;

	int row = index.row();
	if (row >= (int)intLoadDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::InternalLoad>::const_iterator it = intLoadDB.begin();

	int count=0;

	for (; it != intLoadDB.end(); ++it){
		if (it->second.m_category == m_category)
			++count;
		// if count exceeds searched index, stop
		if (count > index.row())
			break;
	}
	Q_ASSERT(it != intLoadDB.end());

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid(m_db->m_schedules)) // for now just check for validity
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

		case Qt::ToolTipRole: {
			if(index.column() == ColCheck) {
				if (!it->second.isValid(m_db->m_schedules))
					return QString::fromStdString(it->second.m_errorMsg);
			}
		}
	}

	return QVariant();
}


QVariant SVDBInternalLoadsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


void SVDBInternalLoadsTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBInternalLoadsTableModel::addNewItem() {
	VICUS::InternalLoad intLoad;
	intLoad.m_displayName.setString(tr("<new internal loads model>").toStdString(), IBK::MultiLanguageString::m_language);

	// set default parameters
	//category dependent

	switch(m_category){
		case VICUS::InternalLoad::IC_Person:{
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_PersonCount, 1);
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_ConvectiveHeatFactor, 0.8);
		}
		break;
		case VICUS::InternalLoad::IC_ElectricEquiment:
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_Power, 0);
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_ConvectiveHeatFactor, 0.8);
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_LatentHeatFactor, 0.0);
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_LossHeatFactor, 0.0);
		break;
		case VICUS::InternalLoad::IC_Lighting:
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_ConvectiveHeatFactor, 0.8);
		break;
		case VICUS::InternalLoad::IC_Other:
			VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_ConvectiveHeatFactor, 0.8);
		break;
		case VICUS::InternalLoad::NUM_MC:
		break;

	}
	intLoad.m_category = m_category;


	intLoad.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_internalLoads.add( intLoad );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBInternalLoadsTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::InternalLoad> & db = m_db->m_internalLoads;
	Q_ASSERT(existingItemIndex.isValid());
	std::map<unsigned int, VICUS::InternalLoad>::const_iterator it = db.begin();

	int count=0;
	for (; it != db.end(); ++it){
		if (it->second.m_category == m_category)
			++count;
		// if count exceeds searched index, stop
		if (count > existingItemIndex.row())
			break;
	}
	Q_ASSERT(it != db.end());

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::InternalLoad newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_internalLoads.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBInternalLoadsTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_internalLoads.remove(id);
	endRemoveRows();
}


void SVDBInternalLoadsTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBInternalLoadsTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBInternalLoadsTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBInternalLoadsTableModel::ColName, QHeaderView::Stretch);
}


void SVDBInternalLoadsTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_internalLoads[id]->m_local = local;
	m_db->m_internalLoads.m_modified = true;
	setItemModified(id);
}


void SVDBInternalLoadsTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBInternalLoadsTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
