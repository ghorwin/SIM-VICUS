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
	if (parent.isValid()) {
		// create model indexes for root categories
		Q_ASSERT((unsigned int)row < m_categoryItems.size());
		// get adress of vector with category-list
		return createIndex(row, column, (void*)(&m_categoryItems[row]));
	}
	return QModelIndex();
}


QModelIndex SVDBConstructionTreeModel::parent(const QModelIndex &index) const {
	return QModelIndex();
}


int SVDBConstructionTreeModel::rowCount(const QModelIndex &parent) const {
	if (!parent.isValid())
		return m_categoryItems.size();
	else {
		return 0;
	}
}


int SVDBConstructionTreeModel::columnCount(const QModelIndex &parent) const {
	if (!parent.isValid())
		return 1;
	else
		return 0;
}


QVariant SVDBConstructionTreeModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	// category item?

	switch (role) {
		case Qt::DisplayRole :
			return m_categoryItems[index.row()].m_categoryName;
	}

	return QVariant();
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
	}

	endResetModel(); // model is complete, views can update
}
