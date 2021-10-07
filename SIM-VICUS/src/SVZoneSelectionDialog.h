/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVZoneSelectionDialogH
#define SVZoneSelectionDialogH

#include <QDialog>

namespace Ui {
class SVZoneSelectionDialog;
}

class SVZoneListModel;
class QSortFilterProxyModel;
class QItemSelection;

/*! A dialog with a list of zone names (with prefixed building and building level names),
	with optional filter functionality.
	User selects a zone and accepts the dialog.

	\note Always construct this dialog anew when using it, because in constructor the zone
			list is updated.
*/
class SVZoneSelectionDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVZoneSelectionDialog(QWidget *parent = nullptr);
	~SVZoneSelectionDialog();

	/*! Here the ID of the selected zone is stored, when the dialog is accepted. */
	unsigned int	m_idZone;

private slots:
	void on_lineEditFilter_textChanged(const QString &pattern);

	void on_listView_doubleClicked(const QModelIndex &index);

	void onZoneListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
	Ui::SVZoneSelectionDialog	*m_ui;
	/*! list model for zone choice*/
	SVZoneListModel				*m_listModel = nullptr;

	/*! filter proxy model for list sorting*/
	QSortFilterProxyModel		*m_proxyModel = nullptr;
};

#endif // SVZoneSelectionDialogH
