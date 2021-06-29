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

#include "QtExt_FilterComboBox.h"

namespace QtExt {

FilterComboBox::FilterComboBox(QWidget* parent) :
	QComboBox(parent),
	m_filterModel(new FilterProxyModel(this)),
	m_model(new QStringListModel(this))
{
	m_filterModel->setSourceModel(m_model);
	connect(this, &FilterComboBox::editTextChanged, this, &FilterComboBox::onEditTextChanged);
	setModel(m_filterModel);
	setEditable(true);
	setCompleter(nullptr);
}

void FilterComboBox::addItem(const QString &text, const QVariant &userData) {
	blockSignals(true);
	QComboBox::addItem(text, userData);
	blockSignals(false);
}

void FilterComboBox::addItem(const QIcon &icon, const QString &text, const QVariant &userData) {
	blockSignals(true);
	QComboBox::addItem(icon, text, userData);
	blockSignals(false);
}

void FilterComboBox::addItems(const QStringList &texts) {
	blockSignals(true);
	QComboBox::addItems(texts);
	blockSignals(false);
}


void FilterComboBox::onEditTextChanged(const QString& text) {
	m_filterModel->setFilter(text);
	showPopup();
}

// FilterProxy

FilterProxyModel::FilterProxyModel(QObject* parent) :
	QSortFilterProxyModel(parent)
{
	setSortCaseSensitivity(Qt::CaseInsensitive);
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
	bool accepted = true;
	QModelIndex sindex = sourceModel()->index(source_row, 0, source_parent);
	if(sindex.isValid()) {
		QString str = sourceModel()->data(sindex).toString();
		accepted = str.contains(m_filter);
	}
	return accepted;
}

void FilterProxyModel::setFilter(const QString& filter) {
	m_filter = filter;
	invalidate();
}

} // namespace QtExt
