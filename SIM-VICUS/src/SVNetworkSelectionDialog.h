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

#ifndef SVNetworkSelectionDialogH
#define SVNetworkSelectionDialogH

#include <QDialog>

namespace Ui {
class SVNetworkSelectionDialog;
}

namespace VICUS {
class GenericNetwork;
}

class QListWidgetItem;

/*! A dialog with a list of network names,
	with optional filter functionality.
	User selects a network and accepts the dialog.
*/
class SVNetworkSelectionDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVNetworkSelectionDialog(QWidget *parent = nullptr);
	~SVNetworkSelectionDialog();

	/*! Updates user interface. */
	void updateUi();

	/*! Here the ID of the selected zone is stored, when the dialog is accepted. */
	unsigned int	m_idNetwork;

private slots:
	void on_listWidget_itemSelectionChanged();

	void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

	void on_comboBoxSupplyType_currentIndexChanged(int index);

private:
	Ui::SVNetworkSelectionDialog	*m_ui;

	/*! Pointer to currently edited boundary condition.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no material to edit.
	*/
	const VICUS::GenericNetwork		*m_current = nullptr;
};

#endif // SVNetworkSelectionDialogH
