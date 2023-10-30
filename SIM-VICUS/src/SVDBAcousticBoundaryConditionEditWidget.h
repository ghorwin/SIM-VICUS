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

#ifndef SVDBAcousticBoundaryConditionEditWidgetH
#define SVDBAcousticBoundaryConditionEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

class QTableWidgetItem;

namespace Ui {
	class SVDBAcousticBoundaryConditionEditWidget;
}

namespace VICUS {
	class AcousticBoundaryCondition;
}

class SVDBAcousticBoundaryConditionTableModel;
class SVDatabase;

/*! Edit widget for boundary condition.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBAcousticBoundaryConditionEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

	enum SoundApsortionColumns {
		ColId,
		ColName,
		ColFraction,
		NumCol
	};


public:
	explicit SVDBAcousticBoundaryConditionEditWidget(QWidget *parent = nullptr);
	~SVDBAcousticBoundaryConditionEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;



private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonColor_colorChanged();

	void on_spinBoxLayerCount_valueChanged(int layerCount);

	/*! Triggered when user modifies a table cell, only needed for 'area fraction'-column. */
	void tableItemChanged(QTableWidgetItem *);

	/*! Slot for react on double click on table cell.*/
	void onCellDoubleClicked(int row, int col);

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	/*! Updates sound absorption layers in table. */
	void updateTable();

	/*! Show a sound absorption database view in order to select a new sound absorption layer
	 *   and change the current selection.
		\param index Index of the layer for sound absorption change.
	*/
	void showSoundAbsorptionSelectionDialog(int index);

	Ui::SVDBAcousticBoundaryConditionEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase									*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBAcousticBoundaryConditionTableModel		*m_dbModel;

	/*! Pointer to currently edited boundary condition.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no material to edit.
	*/
	VICUS::AcousticBoundaryCondition			*m_current;
};

#endif // SVDBAcousticBoundaryConditionEditWidgetH
