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

#include "SVDBZoneTemplateTreeModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_ZoneTemplate.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>
#include <SV_Conversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBZoneTemplateTreeModel::SVDBZoneTemplateTreeModel(QObject * parent, SVDatabase & db) :
	QAbstractItemModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBZoneTemplateTreeModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.column() == ColColor && role == Role_Color) {
		return true;
	}

	// readability improvement
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;

	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it;

	// ZoneTemplate Node?
	if (index.internalPointer() == nullptr) {

		int row = index.row();
		if (row >= (int)db.size())
			return QVariant();

		it = db.begin();
		std::advance(it, row);

		switch (role) {
			case Role_Id :
				return it->first;

			case Qt::DisplayRole : {
				switch (index.column()) {
					case ColId					: return it->first;
					case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
				}
			} break;

			case Qt::DecorationRole : {
				if (index.column() == ColCheck) {
					if (it->second.isValid(m_db->m_internalLoads, m_db->m_zoneControlThermostat,m_db->m_schedules,
										   m_db->m_infiltration, m_db->m_ventilationNatural, m_db->m_zoneIdealHeatingCooling))
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
		}
	}
	else {
		// sub-template

		QModelIndex templateIndex = index.parent();

		int row = templateIndex.row();
		if (row >= (int)db.size())
			return QVariant();

		it = db.begin();
		std::advance(it, row);

		const VICUS::ZoneTemplate & zt = it->second;

		int subTemplateIndex = index.row();
		VICUS::ZoneTemplate::SubTemplateType subType = zt.usedReference((unsigned int)subTemplateIndex);

		switch (role) {
			case Role_SubTemplateType :
				return subType;

			case Role_Id :
				return zt.m_idReferences[subType];

			case Qt::DisplayRole : {
				switch (index.column()) {
					case ColId		: return zt.m_idReferences[subType];
					case ColType	: return VICUS::KeywordListQt::Description("ZoneTemplate::SubTemplateType", subType);
					case ColName	: {
						const VICUS::AbstractDBElement * dbElement = m_db->lookupSubTemplate(subType, zt.m_idReferences);
						if (dbElement == nullptr)
							return tr("<invalid ID reference>");
						else
							return QtExt::MultiLangString2QString(dbElement->m_displayName);
					}
				}
			} break;

			case Qt::BackgroundRole : {
				if (index.column() == ColColor) {
					const VICUS::AbstractDBElement * dbElement = m_db->lookupSubTemplate(subType, zt.m_idReferences);
					if (dbElement != nullptr)
						return dbElement->m_color;
				}
			} break;

			case Qt::DecorationRole : {
				if (index.column() == ColCheck) {
					const VICUS::AbstractDBElement * dbElement = m_db->lookupSubTemplate(subType, zt.m_idReferences);
					if (dbElement != nullptr) {
						bool valid = false;
						switch (subType) {
							case VICUS::ZoneTemplate::ST_IntLoadPerson:
							case VICUS::ZoneTemplate::ST_IntLoadEquipment:
							case VICUS::ZoneTemplate::ST_IntLoadLighting:
							case VICUS::ZoneTemplate::ST_IntLoadOther:
								valid = ((const VICUS::InternalLoad*)dbElement)->isValid(m_db->m_schedules);
							break;
							case VICUS::ZoneTemplate::ST_ControlThermostat:
								valid = ((const VICUS::ZoneControlThermostat*)dbElement)->isValid(m_db->m_schedules);
							break;
							case VICUS::ZoneTemplate::ST_ControlVentilationNatural:
								valid = ((const VICUS::ZoneControlNaturalVentilation*)dbElement)->isValid();
							break;
							case VICUS::ZoneTemplate::ST_Infiltration:
								valid = ((const VICUS::Infiltration*)dbElement)->isValid();
							break;
							case VICUS::ZoneTemplate::ST_VentilationNatural:
								valid = ((const VICUS::VentilationNatural*)dbElement)->isValid(m_db->m_schedules);
							break;
							case VICUS::ZoneTemplate::ST_IdealHeatingCooling:
								valid = ((const VICUS::ZoneIdealHeatingCooling*)dbElement)->isValid();
							break;
							case VICUS::ZoneTemplate::NUM_ST: ; // just to make compiler happy
						}
						if (valid)
							return QIcon("://gfx/actions/16x16/ok.png");
						else
							return QIcon("://gfx/actions/16x16/error.png");
					}
				}
			} break;

		}
	}

	// common handling for templates and sub-templates
	switch (role) {
		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
				case ColColor :
					return QSize(22, 16);

				case ColType : {
					QFontMetrics fm(data(index, Qt::FontRole).value<QFont>());
					QRect bb = fm.boundingRect(data(index, Qt::DisplayRole).toString());
					QSize s(bb.width() + 10, bb.height());
					return s;
				}
			} // switch
			break;

		case Role_BuiltIn :
			return it->second.m_builtIn;
	}

	return QVariant();
}


int SVDBZoneTemplateTreeModel::rowCount ( const QModelIndex & parent) const {
	// top-level - number of zone templates
	if (!parent.isValid())
		return (int)m_db->m_zoneTemplates.size();
	// sub-template nodes to not have children themselves
	if (parent.internalPointer() != nullptr)
		return 0;
	// lookup currently selected zone template
	int row = parent.row();
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT ((unsigned int)row < db.size());

	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, row);

	// return number of assigned sub-templates
	return (int)it->second.subTemplateCount();
}


QVariant SVDBZoneTemplateTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColType				: return tr("Type");
				case ColName				: return tr("Name");
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


QModelIndex SVDBZoneTemplateTreeModel::index(int row, int column, const QModelIndex & parent) const {
	if (!parent.isValid()) {
		return createIndex(row, column, nullptr); // top-level items have a nullptr for identification
	}
	// child item,
	int parentRow = parent.row();
	// take pointer to item
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, parentRow);
	// child items have a pointer to the zone template they belong to as identification
	return createIndex(row, column, (void*)(&it->second));
}


QModelIndex SVDBZoneTemplateTreeModel::parent(const QModelIndex & child) const {
	// nullptr means top-level it
	if (child.internalPointer() == nullptr)
		return QModelIndex();
	else {
		// get internal pointer and lookup item by id
		const VICUS::ZoneTemplate * ptr = reinterpret_cast<const VICUS::ZoneTemplate *>(child.internalPointer());
		// search DB and get the row index of this item
		std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = m_db->m_zoneTemplates.begin();
		int i=0;
		for (; ptr != &it->second && (unsigned int)i<m_db->m_zoneTemplates.size(); ++i, ++it);
		Q_ASSERT((unsigned int)i != m_db->m_zoneTemplates.size());
		return createIndex(i, 0, nullptr);
	}
}


void SVDBZoneTemplateTreeModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneTemplateTreeModel::addNewItem() {
	VICUS::ZoneTemplate c;
	c.m_displayName.setEncodedString("en:<new zone template>");
	c.m_color = SVStyle::randomColor();
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_zoneTemplates.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneTemplateTreeModel::addChildItem(const QModelIndex & templateIndex, int subTemplateType, unsigned int subTemplateID) {
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT(templateIndex.isValid() && templateIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, templateIndex.row());
	// now determine which row needs to be inserted
	int rowIndex = 0;
	for (int i=0; subTemplateType != i && i<VICUS::ZoneTemplate::NUM_ST; ++i) {
		if (it->second.m_idReferences[i] != VICUS::INVALID_ID)
			++rowIndex;
	}
	beginInsertRows(templateIndex, rowIndex, rowIndex);
	VICUS::ZoneTemplate * zt = const_cast<VICUS::ZoneTemplate*>(m_db->m_zoneTemplates[it->second.m_id]);
	Q_ASSERT(zt != nullptr);
	zt->m_idReferences[subTemplateType] = subTemplateID;
	endInsertRows();
	m_db->m_zoneTemplates.m_modified = true;
	return index(rowIndex, 0, templateIndex);
}


QModelIndex SVDBZoneTemplateTreeModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::ZoneTemplate newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_zoneTemplates.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneTemplateTreeModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_zoneTemplates.remove(id);
	endRemoveRows();
}


void SVDBZoneTemplateTreeModel::deleteChildItem(const QModelIndex & templateIndex, int subTemplateType) {
	if (!templateIndex.isValid())
		return;

	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT(templateIndex.isValid() && templateIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, templateIndex.row());
	// now determine which row needs to be removed
	int rowIndex = 0;
	for (int i=0; subTemplateType != i && i<VICUS::ZoneTemplate::NUM_ST; ++i) {
		if (it->second.m_idReferences[i] != VICUS::INVALID_ID)
			++rowIndex;
	}
	// next call updates selection and updates sub-template widget
	// also updates m_currentSubTemplate
	beginRemoveRows(templateIndex, rowIndex, rowIndex);
	VICUS::ZoneTemplate * zt = const_cast<VICUS::ZoneTemplate*>(m_db->m_zoneTemplates[it->second.m_id]);
	Q_ASSERT(zt != nullptr);
	zt->m_idReferences[subTemplateType] = VICUS::INVALID_ID;
	endRemoveRows();
}


void SVDBZoneTemplateTreeModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0, QModelIndex());
	int rows = rowCount(left);
	QModelIndex right = index(rows-1, NumColumns-1, left);
	emit dataChanged(left, right);
}


QModelIndex SVDBZoneTemplateTreeModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0, QModelIndex());
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

