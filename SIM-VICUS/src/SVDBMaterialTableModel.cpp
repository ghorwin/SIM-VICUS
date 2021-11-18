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

#include "SVDBMaterialTableModel.h"

#include <algorithm>

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <VICUS_Material.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_Conversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBMaterialTableModel::SVDBMaterialTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}



QVariant SVDBMaterialTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::Material> & matDB = m_db->m_materials;

	int row = index.row();
	if (row >= (int)matDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::Material>::const_iterator it = matDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
				case ColCategory			:
					try {
						return VICUS::KeywordListQt::Keyword("Material::Category",it->second.m_category);
					} catch (...) {
						return "";
					}
				case ColProductID			: return "--";
				case ColProducer			: return QtExt::MultiLangString2QString(it->second.m_manufacturer);
				case ColSource				: return QtExt::MultiLangString2QString(it->second.m_dataSource);
				case ColRho					: return QString("%L1").arg(it->second.m_para[VICUS::Material::P_Density].value, 0, 'f', 1);
				case ColCet					: return QString("%L1").arg(it->second.m_para[VICUS::Material::P_HeatCapacity].value, 0, 'f', 0);
				case ColLambda				: return QString("%L1").arg(it->second.m_para[VICUS::Material::P_Conductivity].value, 0, 'f', 3);
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid(false)) // for now just check the thermal properties
					return QIcon("://gfx/actions/16x16/ok.png");
				else
					return QIcon("://gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::TextAlignmentRole :
			if (index.column() >= ColRho && index.column() < NumColumns)
				return int(Qt::AlignRight | Qt::AlignVCenter);
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

	}

	return QVariant();
}


int SVDBMaterialTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_materials.size();
}


QVariant SVDBMaterialTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				case ColCategory			: return tr("Category");
				case ColProductID			: return tr("ProductID");
				case ColProducer			: return tr("Producer");
				case ColSource				: return tr("Source");
				case ColRho					: return tr("Rho [kg/m3]");
				case ColCet					: return tr("Ce [J/kgK]");
				case ColLambda				: return tr("Lambda [W/m2K]");

					// TODO : Dirk, andere Spaltenüberschriften
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


void SVDBMaterialTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBMaterialTableModel::addNewItem() {
	VICUS::Material m;
	m.m_displayName.setEncodedString("en:<new material>");

	//set default parameters
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Conductivity, 1);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Density, 1000);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_HeatCapacity, 840);

	m.m_category = VICUS::Material::MC_Bricks;
	m.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_materials.add( m );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBMaterialTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::Material> & db = m_db->m_materials;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::Material>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::Material newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_materials.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBMaterialTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_materials.remove(id);
	endRemoveRows();
}


void SVDBMaterialTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBMaterialTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBMaterialTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBMaterialTableModel::ColName, QHeaderView::Stretch);
}


void SVDBMaterialTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_materials[id]->m_local = local;
	m_db->m_materials.m_modified = true;
	setItemModified(id);
}


void SVDBMaterialTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBMaterialTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}


