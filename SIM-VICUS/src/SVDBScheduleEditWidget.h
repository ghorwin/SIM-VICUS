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

#ifndef SVDBScheduleEditWidgetH
#define SVDBScheduleEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
	class SVDBScheduleEditWidget;
}

#include <NANDRAD_Schedule.h>

namespace VICUS {
	class Schedule;
	class ScheduleInterval;
}

class QListWidgetItem;
class QwtPlotCurve;
class SVDBScheduleTableModel;
class SVDatabase;


/*! Edit widget for schedules.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBScheduleEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:
	explicit SVDBScheduleEditWidget(QWidget *parent = nullptr);
	~SVDBScheduleEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_toolButtonAddPeriod_clicked();
	void on_toolButtonCopyPeriod_clicked();
	void on_toolButtonRemovePeriode_clicked();
	void on_tableWidgetPeriods_currentCellChanged(int currentRow, int , int, int);
	/*! Names of periods can be changed in-place, this function transfers new name
		into data structure.
		\todo remove and use double-click as well?
	*/
	void on_tableWidgetPeriods_cellChanged(int row, int column);
	/*! Double-click on date column (0) opens date/time selection dialog. */
	void on_tableWidgetPeriods_cellDoubleClicked(int row, int column);

	void on_toolButtonBackward_clicked();
	void on_toolButtonAddCurrentDailyCycle_pressed();
	void on_toolButtonForward_clicked();
	void on_toolButtonDeleteCurrentDailyCycle_clicked();

	void on_checkBoxTuesday_toggled(bool checked);
	void on_checkBoxHoliday_toggled(bool checked);
	void on_checkBoxWednesday_toggled(bool checked);
	void on_checkBoxThursday_toggled(bool checked);
	void on_checkBoxFriday_toggled(bool checked);
	void on_checkBoxSaturday_toggled(bool checked);
	void on_checkBoxSunday_toggled(bool checked);
	void on_checkBoxMonday_toggled(bool checked);
	void on_pushButtonSelectWeekDays_clicked();
	void on_pushButtonSelectWeekEnds_clicked();
	void on_radioButtonLinear_toggled(bool checked);

	/*! Triggered, when editing of the current schedule requires re-evaluation of "valid" status and
		update of symbol in table.
	*/
	void onValidityInfoUpdated();


	void on_widgetDailyCycle_dataChanged();

	void on_radioButtonDailyCycles_toggled(bool checked);

	void on_pushButtonPasteAnnualDataFromClipboard_clicked();

	void on_filepathAnnualDataFile_editingFinished();

	void on_radioButtonRelativeFilePath_toggled(bool checked);

	void on_pushButtonEditAnnualDataInTexteditor_clicked();

	void on_listWidgetColumnSelection_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

	void on_radioButtonAnnualScheduleLinear_toggled(bool checked);

private:
	/*! Set up the modified variable of the model to true.
		Also updates the diagram.
	*/
	void modelModify();

	/*! Updates the period table.
		This function is called whenever the interval dataset has been modified (i.e. new periods
		have been added or periods have been removed). Also selects a row in the table.
	*/
	void updatePeriodTable(unsigned int activeRow);

	/*! Updates diagram preview based on currently selected schedule interval, interpolation mode and
		"Full year" checkbox.
	*/
	void updateDailyCycleDiagram();

	/*! Called when a new daily cycle has been selected,i.e. m_currentDailyCycleIndex has changed. */
	void selectDailyCycle();

	/*! Insertes/removes day times in currently edited daily cycle. Also calls updateDailyCycleSelectButtons(). */
	void updateDayTypes(const NANDRAD::Schedule::ScheduledDayType &dt, bool checked);

	/*! Updates enabled/disabled state of buttons based on content of m_currentInterval and current user interface state. */
	void updateDailyCycleSelectButtons();

	/*! Parses the annual tsv-file header and populates the column list using the current file path in the filename line edit.
		Also reselects the column if possible and specified in the file name.
	*/
	void updateColumnIndexList();

	/*! Generates a relative/absolute path from entered annual spline tsv-file path and updates m_current and the respective label.
		Does not modify m_current.
	*/
	void generateRelativeFilePath();

	/*! Updates the preview of the annual spline data. */
	void updateAnnualDataDiagram();


	Ui::SVDBScheduleEditWidget			*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBScheduleTableModel				*m_dbModel;

	/*! Pointer to currently edited schedule.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no schedule to edit.
	*/
	VICUS::Schedule						*m_current;

	/*! Pointer to currently selected interval, updated in on_tableWidgetPeriods_currentCellChanged(). */
	VICUS::ScheduleInterval				*m_currentInterval;
	/*! Index of currently selected daily cycle, initialized with 0 in on_tableWidgetPeriods_currentCellChanged(), modified with arrow buttons. */
	unsigned int						m_currentDailyCycleIndex;

	/*! Actual row index of period table. (-1 no row selected)*/
	int									m_rowIdx=-1;

	/*! Is built in schedule. */
	bool								m_isEditable=true;

	/*! The curve used to plot the preview data. */
	QwtPlotCurve						*m_curve = nullptr;

	/*! The x and y title of plot. */
	std::string							m_xTitle;
	std::string							m_yTitle;
};


#endif // SVDBScheduleEditWidgetH
