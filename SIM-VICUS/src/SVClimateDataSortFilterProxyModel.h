#ifndef SVClimateDataSortFilterProxyModelH
#define SVClimateDataSortFilterProxyModelH

#include <QSortFilterProxyModel>

#include "SVClimateDataTableModel.h"

/*! Sorting/filtering of climate data base. */
class SVClimateDataSortFilterProxyModel : public QSortFilterProxyModel {
public:
	SVClimateDataSortFilterProxyModel(QObject *parent = 0);
	virtual ~SVClimateDataSortFilterProxyModel();

	void setFilterText(const QString & filterText);

	// QSortFilterProxyModel interface
protected:
	bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;
	bool lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const;

private:
	QString m_filterText;
};

#endif // SVClimateDataSortFilterProxyModelH
