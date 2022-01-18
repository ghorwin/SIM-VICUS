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
#include <set>

class QLabel;

namespace Ui {
class SVSmartSelectDialog;
}

namespace VICUS {
	class Object;
	class AbstractDBElement;
}

/*! The dialog for selecting objects based on properties. */
class SVSmartSelectDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSmartSelectDialog(QWidget *parent = nullptr);
	~SVSmartSelectDialog();

	/*! Updates selection options and executes the dialog. */
	void select();

private slots:
	void onSelectClicked();

	void on_comboBoxNodeType_currentIndexChanged(int index);

	void on_checkBoxMaxHeatingDemandBelow_stateChanged(int arg1);

	void on_checkBoxMaxHeatingDemandAbove_stateChanged(int arg1);

	void on_checkBoxLengthBelow_stateChanged(int arg1);

	void on_checkBoxLengthAbove_stateChanged(int arg1);

	void on_pushButtonReset_clicked();

	/*! Triggered when any of the bread crumb buttons has been pressed. */
	void onSelectionButtonPressed();
	/*! Triggered when any of the filter buttons has been pressed. */
	void onOptionButtonPressed();

	void on_checkBoxMaxHeatingDemandEqualTo_stateChanged(int arg1);

	void on_checkBoxNodeDisplayName_stateChanged(int arg1);

private:
	/*! Based on available options and current m_selections info, the button grid is updated. */
	void updateButtonGrid();

	struct FilterOption {
		FilterOption() = default;
		FilterOption(const QString & name, const VICUS::AbstractDBElement * dbElement) :
			m_name(name), m_dbElement(dbElement) {}

		QString							m_name;
		const VICUS::AbstractDBElement	*m_dbElement = nullptr;
		std::vector<FilterOption>		m_options;
	};

	/*! Processes all selected options in the current option and recursivly calls this function until
		all child leaves have been processed. Gathers all objects that are selected by this selection and
		inserts them into the set.
	*/
	void collectSelectedObjects(FilterOption * option, std::set<const VICUS::Object*> & objs);


	Ui::SVSmartSelectDialog		*m_ui;

	/*! The hierarchy of filter options. */
	FilterOption				m_options;

	/*! Contains the currently selected selections in the option hierarchy.
		First index is the index within m_options.m_options[].
	*/
	std::vector<unsigned int>	m_selections;
	/*! The push buttons in the bread crumbs layout (owned). */
	std::vector<QPushButton*>	m_selectionButtons;

	std::vector<QPushButton*>	m_optionButtons;
};


#endif // SVSmartSelectDialogH
