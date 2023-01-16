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

#ifndef SVDBInternalLoadsPersonEditWidgetH
#define SVDBInternalLoadsPersonEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBInternalLoadsPersonEditWidget;
}

namespace VICUS {
	class InternalLoad;
}

class SVDBInternalLoadsTableModel;
class SVDatabase;
class QwtPlotCurve;


class SVDBInternalLoadsPersonEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBInternalLoadsPersonEditWidget(QWidget *parent = nullptr);
	~SVDBInternalLoadsPersonEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxPersonMethod_currentIndexChanged(int index);
	void on_lineEditPersonCount_editingFinishedSuccessfully();
	void on_lineEditConvectiveFactor_editingFinishedSuccessfully();
	void on_pushButtonColor_colorChanged();
	void on_toolButtonSelectOccupancy_clicked();
	void on_toolButtonSelectActivity_clicked();

	void on_toolButtonRemoveOccupancySchedule_clicked();

	void on_toolButtonRemoveActivitySchedule_clicked();

	void on_toolButtonSelectMoistureRate_clicked();

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	void updatePlot();

	Ui::SVDBInternalLoadsPersonEditWidget	*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase								*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBInternalLoadsTableModel				*m_dbModel;

	/*! Pointer to currently edited internal loads model.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no model to edit.
	*/
	VICUS::InternalLoad						*m_current;

	/*! The curve used to plot */
	QwtPlotCurve									*m_curve;

	/*! The data vectors needed for plotting. */
	std::vector<double>								m_xData;
	std::vector<double>								m_yData;

	/*! This functions creates vectors for plotting, could be implemented in VICUS::Schedule as well */
	static void createDataVectorFromSchedule(const VICUS::Schedule &sched,
											 std::vector<double> &time, std::vector<double> &vals);

};

#endif // SVDBInternalLoadsPersonEditWidgetH
