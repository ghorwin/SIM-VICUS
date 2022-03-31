#include <QDebug>


#include "SVPropBuildingZonePropertyTableProxyModel.h"


SVPropBuildingZonePropertyTableProxyModel::SVPropBuildingZonePropertyTableProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
}

bool SVPropBuildingZonePropertyTableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
	bool accepted = true;

	if( !m_nameFilter.isEmpty()) {
		QModelIndex index = sourceModel()->index(sourceRow, 1, sourceParent);
		// ensure that we have a valid index
		Q_ASSERT(index.isValid());

		QString name = sourceModel()->data(index).toString();

		if(!name.contains(m_nameFilter, Qt::CaseInsensitive)) {
			accepted = false;
		}
	}

	return accepted;
}


void SVPropBuildingZonePropertyTableProxyModel::setNameFilter(QString str) {
	m_nameFilter = str;
	invalidate();
}

