/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_FilterComboBox.h"

#include <QKeyEvent>
#include <QtDebug>
#include <QStandardItemModel>
#include <QLineEdit>

namespace QtExt {

FilterComboBox::FilterComboBox(QWidget* parent) :
	QComboBox(parent),
	m_filterModel(new FilterComboBoxProxyModel(this)),
	m_model(new QStandardItemModel(0, 1, this))
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
	lineEdit()->setFocus();
	lineEdit()->grabKeyboard();
}


// FilterComboBoxProxyModel

FilterComboBoxProxyModel::FilterComboBoxProxyModel(QObject* parent) :
	QSortFilterProxyModel(parent)
{
	setSortCaseSensitivity(Qt::CaseInsensitive);
}

bool FilterComboBoxProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
	bool accepted = true;
	QModelIndex sindex = sourceModel()->index(source_row, 0, source_parent);
	if(sindex.isValid()) {
		QString str = sourceModel()->data(sindex).toString();
		accepted = str.contains(m_filter, Qt::CaseInsensitive);
	}
	return accepted;
}


void FilterComboBoxProxyModel::setFilter(const QString& filter) {
	m_filter = filter;
	invalidate();
}


} // namespace QtExt
