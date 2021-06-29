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

#ifndef QtExt_FilterComboBoxH
#define QtExt_FilterComboBoxH

#include <QComboBox>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QStringListModel>

namespace QtExt {

class FilterProxyModel;

class FilterComboBox : public QComboBox
{
	Q_OBJECT
public:
	FilterComboBox(QWidget* parent = nullptr);

	void addItem(const QString &text, const QVariant &userData = QVariant());
	void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant());
	void addItems(const QStringList &texts);

private slots:
	void onEditTextChanged(const QString& text);

private:
	FilterProxyModel*	m_filterModel;
	QStringListModel*	m_model;
};

class FilterProxyModel : public QSortFilterProxyModel {
public:
	FilterProxyModel(QObject* parent = nullptr);

	void setFilter(const QString& filter);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
	QString m_filter;

};

} // namespace QtExt

#endif // QtExt_FilterComboBoxH
