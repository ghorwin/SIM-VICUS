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

#include "SVDBWindowGlazingSystemTableModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_WindowGlazingSystem.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>
#include <SVConversions.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBWindowGlazingSystemTableModel::SVDBWindowGlazingSystemTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBWindowGlazingSystemTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.column() == ColColor && role == Role_Color) {
		return true;
	}

	// readability improvement
	const VICUS::Database<VICUS::WindowGlazingSystem> & winDB = m_db->m_windowGlazingSystems;

	int row = index.row();
	if (row >= (int)winDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::WindowGlazingSystem>::const_iterator it = winDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QtExt::MultiLangString2QString(it->second.m_displayName);
				//case ColType				: return VICUS::KeywordListQt::Description("Window::WindowType", it->second.m_type);
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
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
	}

	return QVariant();
}


int SVDBWindowGlazingSystemTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_windowGlazingSystems.size();
}


QVariant SVDBWindowGlazingSystemTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
			//	case ColType				: return tr("Type");
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


void SVDBWindowGlazingSystemTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBWindowGlazingSystemTableModel::addNewItem() {
	VICUS::WindowGlazingSystem c;
	c.m_displayName.setString(tr("<new window glazing system>").toStdString(), IBK::MultiLanguageString::m_language);
	c.m_color = SVStyle::randomColor();
	VICUS::KeywordList::setParameter(c.m_para, "WindowGlazingSystem::para_t", VICUS::WindowGlazingSystem::P_ThermalTransmittance, 1);
	c.m_modelType = VICUS::WindowGlazingSystem::MT_Simple;

	NANDRAD::LinearSplineParameter &spline = c.m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC];
	spline.m_name  =VICUS::KeywordList::Keyword("WindowGlazingSystem::splinePara_t",VICUS::WindowGlazingSystem::SP_SHGC);
	spline.m_xUnit = IBK::Unit("Deg");
	spline.m_yUnit = IBK::Unit("---");

	spline.m_interpolationMethod = NANDRAD::LinearSplineParameter::I_LINEAR;
	spline.m_values.setValues(std::vector<double>{0, 10, 20, 30, 40, 50, 60, 70, 80, 90},
							  std::vector<double>{.6, .6, .6, .59,.57,.45,.3, .2, .1,0.0});

	std::string errorMsg;
	spline.m_values.makeSpline(errorMsg);

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_windowGlazingSystems.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBWindowGlazingSystemTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::WindowGlazingSystem> & db = m_db->m_windowGlazingSystems;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::WindowGlazingSystem>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::WindowGlazingSystem newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_windowGlazingSystems.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBWindowGlazingSystemTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_windowGlazingSystems.remove(id);
	endRemoveRows();
}


void SVDBWindowGlazingSystemTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBWindowGlazingSystemTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBWindowGlazingSystemTableModel::ColColor, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBWindowGlazingSystemTableModel::ColName, QHeaderView::Stretch);
}


void SVDBWindowGlazingSystemTableModel::setItemLocal(const QModelIndex &index, bool local)
{
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	m_db->m_windowGlazingSystems[id]->m_local = local;
	m_db->m_windowGlazingSystems.m_modified = true;
	setItemModified(id);
}


void SVDBWindowGlazingSystemTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBWindowGlazingSystemTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

