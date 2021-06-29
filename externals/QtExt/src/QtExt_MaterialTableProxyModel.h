/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#ifndef QtExt_MaterialTableProxyModelH
#define QtExt_MaterialTableProxyModelH

#include <QSortFilterProxyModel>


namespace QtExt {

class MaterialBase;

/*! Implementation of a proxy model between MaterialTableModel and
	the TableView of the MaterialDatabaseWidget
	*/
class MaterialTableProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	/*! Constructor*/
	MaterialTableProxyModel(QObject *parent = 0);

	/*! Returns the row of the material with the given id.*/
	int getRow(int id) const;

protected:
	/*! Re-implemented for alternative row colors. */
	virtual QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const;

	/*! own implementation of lessThan in order to sort the material according to the clicked column*/
	virtual bool lessThan( const QModelIndex & left, const QModelIndex & right ) const;

	/*! for filtering according to MaterialCategory or a part of the name\n
		see also MaterialTableProxyModel::setCategoryFilter and MaterialTableProxyModel::setNameFilter*/
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

	/*! creates a new numbering of the rows*/
	QVariant headerData( int section, Qt::Orientation orientation, int role) const;

signals:

public slots:
	/*! receives from MaterialDatabaseDialog::sendCategoryIndex*/
	void setCategoryFilter(int);

	/*! Set a new string for material name filter. Set empty string for no filtering.*/
	void setNameFilter(QString);

	/*! Set a new string for material producer filter. Set empty string for no filtering.*/
	void setProducerFilter(QString);

	/*! Set a new string for material source filter. Set empty string for no filtering.*/
	void setSourceFilter(QString);

	/*! Set a new string for comment filter. Set empty string for no filtering.*/
	void setCommentFilter(QString);

	/*! Sets a new filter for showing deprecated materials.	*/
	void setShowDeprecatedFilter(bool showDeprecated);

	/*! Sets a new capability combination as filter criterion, set to 0 to disable this filter.
	*/
	void setCapabilityFilter(int capabilities);

private:
	/*! index for the category filter (see MaterialCategory).
		is -1 of nothing to filter
	*/
	int m_categoryFilter;

	/*! String for material name filter.
		if string is empty, no filtering\n
		a '*' is set at begin and end of the string for correct wildcard filtering (see filterAcceptsRow)
	*/
	QString m_nameFilter;

	/*! String for material producer filter.
		if string is empty, no filtering\n
		a '*' is set at begin and end of the string for correct wildcard filtering (see filterAcceptsRow)
	*/
	QString m_producerFilter;

	/*! String for material source filter.
		if string is empty, no filtering\n
		a '*' is set at begin and end of the string for correct wildcard filtering (see filterAcceptsRow)
	*/
	QString m_sourceFilter;

	/*! String for comment filter.
		if string is empty, no filtering\n
		a '*' is set at begin and end of the string for correct wildcard filtering (see filterAcceptsRow)
	*/
	QString m_commentFilter;

	/*! If true also deprecated or removed materials will be shown.*/
	bool	m_showDeprecatedFilter;

	/*! Allows filtering for material capabilities.
		The number can contain the following values:
		- 1 - thermal
		- 2 - vapor
		- 4 - liquid
		- 8 - air
		- 16 - salt
		- 32 - VOC
		The number can be a combination of all of the above values.
	*/
	int		m_capabilityFilter;
};

}

/*! @file QtExt_MaterialTableProxyModel.h
	@brief Contains the class MaterialTableProxyModel.
*/

#endif // QtExt_MaterialTableProxyModelH
