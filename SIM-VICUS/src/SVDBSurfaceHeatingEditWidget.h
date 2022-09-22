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

#ifndef SVDBSurfaceHeatingEditWidgetH
#define SVDBSurfaceHeatingEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBSurfaceHeatingEditWidget;
}

namespace VICUS {
	class SurfaceHeating;
}

class SVDBSurfaceHeatingTableModel;
class SVDatabase;
class QTableWidgetItem;
class QwtPlotCurve;

class SVDBSurfaceHeatingEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBSurfaceHeatingEditWidget(QWidget *parent = nullptr);
	~SVDBSurfaceHeatingEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxType_currentIndexChanged(int index);
	void on_pushButtonColor_colorChanged();

	void on_lineEditPipeSpacing_editingFinishedSuccessfully();

	void on_lineEditFluidVelocity_editingFinishedSuccessfully();

	void on_lineEditTemperaturDifference_editingFinishedSuccessfully();

	void on_lineEditHeatingLimit_editingFinishedSuccessfully();

	void on_lineEditCoolingLimit_editingFinishedSuccessfully();

	void on_toolButtonSelectPipes_clicked();

	void on_tableWidgetCooling_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

	void on_tableWidgetCooling_itemChanged(QTableWidgetItem *item);

	void on_tableWidgetHeating_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);

	void on_tableWidgetHeating_itemChanged(QTableWidgetItem *item);

private:

	/*! Set up the modified variable of the model to true. */
	void modelModify();

	void updatePlot();

	Ui::SVDBSurfaceHeatingEditWidget				*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase										*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBSurfaceHeatingTableModel					*m_dbModel;

	/*! Pointer to currently edited zone control SurfaceHeating model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::SurfaceHeating							*m_current;

	/*! The curve used to plot */
	QwtPlotCurve									*m_curveHeating;
	QwtPlotCurve									*m_curveCooling;

	/*! The data vectors needed for plotting. */
	std::vector<double>								m_xDataHeating;
	std::vector<double>								m_yDataHeating;
	std::vector<double>								m_xDataCooling;
	std::vector<double>								m_yDataCooling;
};

#endif // SVDBSurfaceHeatingEditWidgetH
