#ifndef SVPropBuildingZonePropertyTableProxyModelH
#define SVPropBuildingZonePropertyTableProxyModelH

#include <QSortFilterProxyModel>


/*! Implementation of a proxy model between MaterialTableModel and
	the TableView of the MaterialDatabaseWidget
	*/
class SVPropBuildingZonePropertyTableProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	/*! Constructor*/
	SVPropBuildingZonePropertyTableProxyModel(QObject *parent = nullptr);

protected:
	/*! for filtering according to MaterialCategory or a part of the name\n
		see also SVPropBuildingZonePropertyTableProxyModel::setCategoryFilter and SVPropBuildingZonePropertyTableProxyModel::setNameFilter*/
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

public slots:
	/*! Set a new string for room name filter. Set empty string for no filtering.*/
	void setNameFilter(QString);

private:
	/*! String for room name filter.
		if string is empty, no filtering\n
		a '*' is set at begin and end of the string for correct wildcard filtering (see filterAcceptsRow)
	*/
	QString m_nameFilter;

};


/*! @file SVPropBuildingZonePropertyTableProxyModel.h
	@brief Contains the class SVPropBuildingZonePropertyTableProxyModel.
*/

#endif // SVPropBuildingZonePropertyTableProxyModelH
