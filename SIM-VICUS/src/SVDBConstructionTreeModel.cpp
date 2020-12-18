#include "SVDBConstructionTreeModel.h"

#include <VICUS_KeywordList.h>

SVDBConstructionTreeModel::SVDBConstructionTreeModel(QObject *parent)
	: QAbstractItemModel(parent)
{
}


QVariant SVDBConstructionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
	return QVariant();
}


QModelIndex SVDBConstructionTreeModel::index(int row, int column, const QModelIndex &parent) const {
	if (!parent.isValid()) {
		// get adress of vector with category-list
		return createIndex(row, column, nullptr); // category items have a nullptr as internal pointer
	}
	else {
		// create model indexes for root categories
		Q_ASSERT((unsigned int)parent.row() < m_categoryItems.size());
		// get adress of vector with category-list
		return createIndex(row, column, (void*)(&m_categoryItems[parent.row()])); // leaf nodes have the vector of their category as internal pointer
	}
	return QModelIndex();
}


QModelIndex SVDBConstructionTreeModel::parent(const QModelIndex &index) const {
	if (index.internalPointer() == nullptr)
		return QModelIndex();
	else {
		CategoryItem * catItem = reinterpret_cast<CategoryItem *>(index.internalPointer());
		// find out, which row this item belongs to
		int row = catItem - m_categoryItems.data();
		return createIndex(row, 0, nullptr);
	}
}


int SVDBConstructionTreeModel::rowCount(const QModelIndex &parent) const {
	if (!parent.isValid())
		return m_categoryItems.size();
	else {
		// only second level leaves have childs, hence the parent of the parent must be invalid
		if (!parent.parent().isValid())
			return m_categoryItems[parent.row()].m_constructions.size();
		else
			return 0;
	}
}


int SVDBConstructionTreeModel::columnCount(const QModelIndex &parent) const {
	return NUM_C;
}


QVariant SVDBConstructionTreeModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	// category item?
	if (index.internalPointer() == nullptr) {
		switch (role) {
			case Qt::DisplayRole :
				if (index.column()==0)
					return m_categoryItems[index.row()].m_categoryName;
		}
	}
	else {
		// get vector with construction instances
		CategoryItem * catItem = reinterpret_cast<CategoryItem *>(index.internalPointer());
		unsigned int constructionId = catItem->m_constructions[index.row()];
		// now get the construction data
		Q_ASSERT(m_dbConstructions->find(constructionId) != m_dbConstructions->end());
		const VICUS::Construction * con = &m_dbConstructions->at(constructionId);
		switch (role) {
			case Qt::DisplayRole :
				switch (index.column()) {
					case C_Name		: return con->m_displayName;
					case C_UValue	: return "<todo: uvalue here>";
				}
			default:;
		}

	}

	return QVariant();
}


Qt::ItemFlags SVDBConstructionTreeModel::flags(const QModelIndex & index) const {
	if (!index.isValid())
		return Qt::ItemFlags();

	// category item?
	if (index.internalPointer() == nullptr) {
		return Qt::ItemIsEnabled;
	}
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


void SVDBConstructionTreeModel::setDataStore(std::map<unsigned int, VICUS::Construction> & dbConstructions) {
	beginResetModel(); // signal that a complete rebuild of the model starts
	m_dbConstructions = &dbConstructions;

	// build up internal cache
	std::map<VICUS::Construction::UsageType, std::vector<unsigned int> > categories;

	for (auto & c : *m_dbConstructions)
		categories[c.second.m_usageType].push_back( c.first );

	// copy to local data structure
	m_categoryItems.clear();
	for (auto & c : categories) {
		CategoryItem catItem;
		catItem.m_usageType = c.first;
		catItem.m_constructions = c.second;
		catItem.m_categoryName = QString::fromStdString(VICUS::KeywordList::Description("Construction::UsageType", c.first));
		m_categoryItems.emplace_back(catItem);
	}

	endResetModel(); // model is complete, views can update
}


