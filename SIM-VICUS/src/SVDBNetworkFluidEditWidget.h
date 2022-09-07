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

#ifndef SVDBNetworkFluidEditWidgetH
#define SVDBNetworkFluidEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBNetworkFluidEditWidget;
}

namespace VICUS {
	class NetworkFluid;
}

class SVDBNetworkFluidTableModel;
class SVDatabase;

/*! Edit widget for fluids */
class SVDBNetworkFluidEditWidget :  public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBNetworkFluidEditWidget(QWidget *parent = nullptr);
	~SVDBNetworkFluidEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonColor_colorChanged();

	void on_lineEditDensity_editingFinished();
	void on_lineEditHeatCapacity_editingFinished();
	void on_lineEditThermalConductivity_editingFinished();

	void on_toolButtonAddViscosityPoint_clicked();

	void on_toolButtonRemoveViscosityPoint_clicked();

	void on_tableWidgetViscosity_cellChanged(int row, int column);

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	void modifyViscosity();

	Ui::SVDBNetworkFluidEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBNetworkFluidTableModel			*m_dbModel;

	/*! Pointer to currently edited item.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no item to edit.
	*/
	VICUS::NetworkFluid					*m_currentFluid;
};


#endif // SVDBNetworkFluidEditWidgetH
