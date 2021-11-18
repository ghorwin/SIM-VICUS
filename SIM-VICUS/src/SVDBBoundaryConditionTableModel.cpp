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

#include "SVDBBoundaryConditionTableModel.h"

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <VICUS_BoundaryCondition.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <NANDRAD_KeywordList.h>

#include <QtExt_Conversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBBoundaryConditionTableModel::SVDBBoundaryConditionTableModel(QObject *parent, SVDatabase &db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBBoundaryConditionTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.column() == ColColor && role == Role_Color) {
		return true;
	}

	// readability improvement
	const VICUS::Database<VICUS::BoundaryCondition> & bcDB = m_db->m_boundaryConditions;

	int row = index.row();
	if (row >= (int)bcDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::BoundaryCondition>::const_iterator it = bcDB.begin();
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
				if (it->second.isValid(m_db->m_schedules))
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

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;

		case Role_Local :
			return it->second.m_local;
	}

	return QVariant();
}


int SVDBBoundaryConditionTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_boundaryConditions.size();
}


QVariant SVDBBoundaryConditionTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


void SVDBBoundaryConditionTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBBoundaryConditionTableModel::addNewItem() {
	VICUS::BoundaryCondition bc;
	bc.m_displayName.setEncodedString("en:<new boundary condition>");
	bc.m_color = SVStyle::randomColor();

	//set default parameters
	bc.m_heatConduction.m_modelType = VICUS::InterfaceHeatConduction::MT_Constant;
	NANDRAD::KeywordList::setParameter(bc.m_heatConduction.m_para, "InterfaceHeatConduction::para_t", VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient, 8);

	bc.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_Constant;
	NANDRAD::KeywordList::setParameter(bc.m_solarAbsorption.m_para, "InterfaceSolarAbsorption::para_t", NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient, 0.6);

	bc.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_Constant;
	NANDRAD::KeywordList::setParameter(bc.m_longWaveEmission.m_para, "InterfaceLongWaveEmission::para_t", NANDRAD::InterfaceLongWaveEmission::P_Emissivity, 0.9);

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_boundaryConditions.add( bc );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBBoundaryConditionTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::BoundaryCondition> & db = m_db->m_boundaryConditions;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::BoundaryCondition>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::BoundaryCondition newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_boundaryConditions.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBBoundaryConditionTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_boundaryConditions.remove(id);
	endRemoveRows();
}


void SVDBBoundaryConditionTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBBoundaryConditionTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBBoundaryConditionTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBBoundaryConditionTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBBoundaryConditionTableModel::ColName, QHeaderView::Stretch);
}


void SVDBBoundaryConditionTableModel::setItemLocal(const QModelIndex &index, bool local) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_boundaryConditions[id]->m_local = local;
	m_db->m_boundaryConditions.m_modified = true;
	setItemModified(id);
}


void SVDBBoundaryConditionTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBBoundaryConditionTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
