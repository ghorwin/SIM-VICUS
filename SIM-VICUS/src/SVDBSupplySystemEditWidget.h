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

#ifndef SVDBSupplySystemEditWidgetH
#define SVDBSupplySystemEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"
#include "ui_SVDBSupplySystemEditWidget.h"

//namespace Ui {
//	class SVDBSupplySystemEditWidget;
//}

namespace VICUS {
	class SupplySystem;
}

class SVDBSupplySystemTableModel;
class SVDatabase;

/*! Edit widget for boundary condition.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBSupplySystemEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBSupplySystemEditWidget(QWidget *parent = nullptr);
	~SVDBSupplySystemEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;


private slots:
	void on_lineEditName_editingFinished();
	void on_lineEditMaxMassFlux_editingFinished();
	void on_lineEditSupplyTemp_editingFinished();
	void on_lineEditMaxMassFluxFMU_editingFinished();
	void on_lineEditHeatingPowerFMU_editingFinished();
	void on_pushButtonFMUPath_clicked();
	void on_comboBoxSupplyType_currentIndexChanged(int index);

	void on_pushButtonColor_colorChanged();

	void on_supplyFMUFile_editingFinished();


private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBSupplySystemEditWidget	*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase						*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBSupplySystemTableModel		*m_dbModel;

	/*! Pointer to currently edited boundary condition.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no material to edit.
	*/
	VICUS::SupplySystem				*m_current;
};

#endif // SVDBSupplySystemEditWidgetH
