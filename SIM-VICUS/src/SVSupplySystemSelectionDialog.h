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

#ifndef SVSupplySystemSelectionDialogH
#define SVSupplySystemSelectionDialogH

#include <QDialog>

namespace Ui {
	class SVSupplySystemSelectionDialog;
}

namespace VICUS {
class SupplySystem;
}

class QListWidgetItem;

/*! A dialog with a list of network names,
	with optional filter functionality.
	User selects a network and accepts the dialog.
*/
class SVSupplySystemSelectionDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSupplySystemSelectionDialog(QWidget *parent = nullptr);
	~SVSupplySystemSelectionDialog();

	/*! Updates complete dialog. */
	void updateUi();

	/*! Updates current supply. */
	void updateCurrent();

	/*! Returns external supply object id, if an object is chosen, INVALID_ID otherwise. */
	unsigned int SupplySystemId();

private slots:
	void on_listWidgetSupply_itemSelectionChanged();

	void on_listWidgetSupply_itemDoubleClicked(QListWidgetItem *item);

	void on_comboBoxSupplyType_currentIndexChanged(int index);

	void on_doubleSpinBoxMaxMassFlux_valueChanged(double arg1);

	void on_doubleSpinBoxSupplyTemp_valueChanged(double arg1);

	void on_pushButtonFMUPath_clicked();

	void on_doubleSpinBoxMaxMassFluxFMU_valueChanged(double arg1);

	void on_doubleSpinBoxHeatingPowerFMU_valueChanged(double arg1);

	void on_pushButtonCreateNew_clicked();

private:
	Ui::SVSupplySystemSelectionDialog	*m_ui;

	/*! Pointer to currently edited supply description stucture.
	*/
	const VICUS::SupplySystem		*m_current = nullptr;
};

#endif // SVSupplySystemSelectionDialogH
