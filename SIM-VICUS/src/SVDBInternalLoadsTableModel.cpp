#include "SVDBInternalLoadsTableModel.h"


#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"
#include "SVDBInternalLoadsPersonEditDialog.h"

SVDBInternalLoadsTableModel::SVDBInternalLoadsTableModel(QObject * parent, SVDatabase & db, Type t) :
	QAbstractTableModel(parent),
	m_db(&db)
{
	// must only be created from SVDBInternalLoadsPersonEditDialog
	///TODO Dirk muss umgestellt werden wenn wir mehrere Modelle erstellen (Person, ElectricEquipment, Ligthing,...)
	switch (t) {
		case T_Person:				Q_ASSERT(dynamic_cast<SVDBInternalLoadsPersonEditDialog*>(parent) != nullptr); break;
		//case T_ElectricEquipment:	Q_ASSERT(dynamic_cast<SVDB...*>(parent) != nullptr); break;
		//case T_Ligthing:			Q_ASSERT(dynamic_cast<SVDB...*>(parent) != nullptr); break;
		//case T_Other:				Q_ASSERT(dynamic_cast<SVDB...*>(parent) != nullptr); break;
		case NUM_T:					Q_ASSERT(t != NUM_T);
	}
	Q_ASSERT(m_db != nullptr);
}


SVDBInternalLoadsTableModel::~SVDBInternalLoadsTableModel() {
}

int SVDBInternalLoadsTableModel::columnCount ( const QModelIndex & ) const {
	return NumColumns;
}

int SVDBInternalLoadsTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_internalLoads.size();
}

QModelIndex SVDBInternalLoadsTableModel::addNewItem(VICUS::InternalLoad intLoad) {
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_internalLoads.add( intLoad );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


bool SVDBInternalLoadsTableModel::deleteItem(QModelIndex index) {
	if (!index.isValid())
		return false;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_internalLoads.remove(id);
	endRemoveRows();
	return true;
}


void SVDBInternalLoadsTableModel::resetModel() {
	beginResetModel();
	endResetModel();
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

QVariant SVDBInternalLoadsTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::InternalLoad> & intLoadDB = m_db->m_internalLoads;

	int row = index.row();
	if (row >= (int)intLoadDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::InternalLoad>::const_iterator it = intLoadDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			// Note: when accessing multilanguage strings below, take name in current language or if missing, "all"
			std::string langId = QtExt::LanguageHandler::instance().langId().toStdString();
			std::string fallBackLangId = "en";

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QString::fromStdString(it->second.m_displayName.string(langId, fallBackLangId));
//				case ColCategory			:
//					try {
//						return VICUS::KeywordListQt::Keyword("Material::Category",it->second.m_category);
//					} catch (...) {
//						return "";
//					}
				case ColSource				: return QString::fromStdString(it->second.m_dataSource.string(langId, fallBackLangId));
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid()) // for now just check for validity
					return QIcon("://gfx/actions/16x16/ok.png");
				else
					return QIcon("://gfx/actions/16x16/error.png");
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
				//case ColCategory			: return tr("Category");
				case ColSource				: return tr("Source");
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


QModelIndex SVDBInternalLoadsTableModel::addNewItem() {
	VICUS::InternalLoad intLoad;
	intLoad.m_displayName.setEncodedString("en:<new internal load model>");

	//set default parameters
	VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_PersonCount, 1);
	VICUS::KeywordList::setParameter(intLoad.m_para, "InternalLoad::para_t", VICUS::InternalLoad::P_ConvectiveHeatFactor, 0.8);

	intLoad.m_category = VICUS::InternalLoad::IC_Person;

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_internalLoads.add( intLoad );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}

