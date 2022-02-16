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

#ifndef QtExt_FilterComboBoxH
#define QtExt_FilterComboBoxH

#include <QComboBox>
#include <QSortFilterProxyModel>

class QStandardItemModel;

namespace QtExt {

class FilterComboBoxProxyModel;
class ComboLineEdit;

/*! A specialized combo box that shows a list of filtered options while typing
	in the line edit.
*/
class FilterComboBox : public QComboBox {
	Q_OBJECT
public:
	FilterComboBox(QWidget* parent = nullptr);

	void addItem(const QString &text, const QVariant &userData = QVariant());
	void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant());
	void addItems(const QStringList &texts);

private slots:
	void onEditTextChanged(const QString& text);

private:
	FilterComboBoxProxyModel*	m_filterModel;
	QStandardItemModel*	m_model;
};

/*! The filter proxy model that is used by the filter combo box. */
class FilterComboBoxProxyModel : public QSortFilterProxyModel {
public:
	FilterComboBoxProxyModel(QObject* parent = nullptr);

	void setFilter(const QString& filter);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
	QString m_filter;
};


} // namespace QtExt

#endif // QtExt_FilterComboBoxH
