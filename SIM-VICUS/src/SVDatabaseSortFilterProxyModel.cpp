#include "SVDatabaseSortFilterProxyModel.h"

#include "SVConstants.h"

SVDatabaseSortFilterProxyModel::SVDatabaseSortFilterProxyModel(QObject * parent) :
	QSortFilterProxyModel(parent)
{

}


SVDatabaseSortFilterProxyModel::~SVDatabaseSortFilterProxyModel() {
	}


void SVDatabaseSortFilterProxyModel::setFilterText(const QString & filterText) {
	beginResetModel();
	m_filterText = filterText;
	endResetModel();
}


bool SVDatabaseSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex & /*source_parent*/) const {
	if (filterRegExp().isEmpty())
		return true; // no filter, always accepted

	// search through all text columns, if any of the texts in the first columns contains the filter text,
	return false;
}


bool SVDatabaseSortFilterProxyModel::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const {
	// different comparison operations
	return source_left.data().toString() < source_right.data().toString();
}
