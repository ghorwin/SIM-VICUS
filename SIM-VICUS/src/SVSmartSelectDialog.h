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

#ifndef SVSmartSelectDialogH
#define SVSmartSelectDialogH

#include <QDialog>

namespace Ui {
class SVSmartSelectDialog;
}

/*! The dialog for selecting objects based on properties. */
class SVSmartSelectDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSmartSelectDialog(QWidget *parent = nullptr);
	~SVSmartSelectDialog();

private slots:
	void onSelectClicked();

	void on_comboBoxNodeType_currentIndexChanged(int index);

	void on_checkBoxMaxHeatingDemandBelow_stateChanged(int arg1);

	void on_checkBoxMaxHeatingDemandAbove_stateChanged(int arg1);

	void on_checkBoxLengthBelow_stateChanged(int arg1);

	void on_checkBoxLengthAbove_stateChanged(int arg1);

private:
	Ui::SVSmartSelectDialog *m_ui;
};

#endif // SVSmartSelectDialogH
