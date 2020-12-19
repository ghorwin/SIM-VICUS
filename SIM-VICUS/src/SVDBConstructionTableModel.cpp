#include "SVDBConstructionTableModel.h"

#include <map>
#include <algorithm>

#include <QtExt_varianthelper.h>

#include <QIcon>

#include <VICUS_Construction.h>
#include <VICUS_Database.h>

DBConstructionTableModel::DBConstructionTableModel(QObject * parent, Database & db) :
	QAbstractTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


DBConstructionTableModel::~DBConstructionTableModel() {
}


int DBConstructionTableModel::columnCount ( const QModelIndex & ) const {
	return NumColumns;
}


QVariant DBConstructionTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readibility improvement
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
				case ColName				: return QString::fromStdString(it->second.m_displayName.string("de", true)); // Note: take name in current language or if missing, "all"
//				case ColInsulationKind		: return TH::ConstructionType::insulationKindString(it->second.m_insulationKind);
//				case ColMaterialKind		: return TH::ConstructionType::materialKindString(it->second.m_materialKind);
//				case ColConstructionKind	: return TH::ConstructionType::constructionKindString(it->second.m_constructionKind);
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
					return QIcon("://gfx/actions/16x16/ok.png");
				else
					return QIcon("://gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::TextAlignmentRole :
			switch (index.column()) {
				case ColUValue				: return int(Qt::AlignRight | Qt::AlignVCenter);
				case ColNumLayers			: return int(Qt::AlignRight | Qt::AlignVCenter);
			}
			break;

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;

		case Role_RawPointer :
			return QtExt::VPtr<const VICUS::Construction>::asQVariant(&it->second);
	}

	return QVariant();
}


int DBConstructionTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_constructions.size();
}


QVariant DBConstructionTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical || role != Qt::DisplayRole)
		return QVariant();
	switch ( section ) {
		case ColId					: return tr("Id");
		case ColName				: return tr("Name");
		case ColInsulationKind		: return tr("Insulation");
		case ColMaterialKind		: return tr("Material");
		case ColConstructionKind	: return tr("Construction");
		case ColNumLayers			: return tr("Layers");
		case ColUValue				: return tr("U [W/m2K]");
		default: ;
	}
	return QVariant();
}


QModelIndex DBConstructionTableModel::addNewItem() {
	VICUS::Construction c;
	c.m_displayName.setEncodedString("en:<new construction type>");
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_constructions.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex DBConstructionTableModel::addNewItem(VICUS::Construction c) {
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_constructions.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


bool DBConstructionTableModel::deleteItem(QModelIndex index) {
	if (!index.isValid())
		return false;
	unsigned int id = data(index, Qt::UserRole).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_constructions.remove(id);
	endRemoveRows();
	return true;
}


void DBConstructionTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


void DBConstructionTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex DBConstructionTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Qt::UserRole).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}

