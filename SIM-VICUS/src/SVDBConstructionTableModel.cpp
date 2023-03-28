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

#include "SVDBConstructionTableModel.h"

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

#include "SVConstants.h"
#include "SVStyle.h"

SVDBConstructionTableModel::SVDBConstructionTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBConstructionTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::Construction> & conDB = m_db->m_constructions;

	int row = index.row();
	if (row >= (int)conDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::Construction>::const_iterator it = conDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
                case ColDataSource          : return QtExt::MultiLangString2QString(it->second.m_dataSource);
				case ColUsageType			: return VICUS::KeywordListQt::Description("Construction::UsageType", it->second.m_usageType);
				case ColInsulationKind		: return VICUS::KeywordListQt::Description("Construction::InsulationKind", it->second.m_insulationKind);
				case ColMaterialKind		: return VICUS::KeywordListQt::Description("Construction::MaterialKind", it->second.m_materialKind);
				case ColNumLayers			: return QString("%1").arg(it->second.m_materialLayers.size());
				case ColUValue				: {
					double UValue;
					bool validUValue = it->second.calculateUValue(UValue, m_db->m_materials, 0.13, 0.04);
					if (validUValue)
						return QString("%L1").arg(UValue, 0, 'f', 3);
					else
						return tr("---");
				}
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid(m_db->m_materials))
					return QIcon(":/gfx/actions/16x16/ok.png");
				else
					return QIcon(":/gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::TextAlignmentRole :
			switch (index.column()) {
				case ColUValue				: return int(Qt::AlignRight | Qt::AlignVCenter);
				case ColNumLayers			: return int(Qt::AlignRight | Qt::AlignVCenter);
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
		case Qt::ToolTipRole: {
			if(index.column() == ColCheck) {
				std::string errorMsg = "";
				if (!it->second.isValid(m_db->m_materials))
					return QString::fromStdString(it->second.m_errorMsg);
			}
		}
	}

	return QVariant();
}


int SVDBConstructionTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_constructions.size();
}


QVariant SVDBConstructionTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
                case ColDataSource          : return tr("Source");
                case ColInsulationKind		: return tr("Insulation");
				case ColMaterialKind		: return tr("Material");
				case ColUsageType			: return tr("Usage");
				case ColNumLayers			: return tr("Layers");
				case ColUValue				: return tr("U [W/m2K]");
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


void SVDBConstructionTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBConstructionTableModel::addNewItem() {
	VICUS::Construction c;
	c.m_displayName.setEncodedString("en:<new construction type>");

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_constructions.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBConstructionTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::Construction> & db = m_db->m_constructions;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::Construction>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::Construction newItem(it->second);
	unsigned int id = m_db->m_constructions.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBConstructionTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_constructions.remove(id);
	endRemoveRows();
}



void SVDBConstructionTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColName, QHeaderView::Stretch);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColNumLayers, QHeaderView::ResizeToContents);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBConstructionTableModel::ColUValue, QHeaderView::ResizeToContents);
}


void SVDBConstructionTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_constructions[id]->m_local = local;
	m_db->m_constructions.m_modified = true;
	setItemModified(id);
}


void SVDBConstructionTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBConstructionTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

