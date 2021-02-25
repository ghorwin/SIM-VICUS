#ifndef SVDBScheduleEditWidget_H
#define SVDBScheduleEditWidget_H

#include <QWidget>

#include <NANDRAD_Schedule.h>

namespace VICUS {
	class Schedule;
	class ScheduleInterval;
}

class SVDBScheduleTableModel;
class SVDatabase;


namespace Ui {
class SVDBScheduleEditWidget;
}
/*! Edit widget for schedules.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBScheduleEditWidget : public QWidget{
	Q_OBJECT

public:
	explicit SVDBScheduleEditWidget(QWidget *parent = nullptr);
	~SVDBScheduleEditWidget();

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBScheduleTableModel * dbModel);

	/*! Sets up the widget for a Schedule with a given ID. */
	void updateInput(int id);

private slots:

	void on_toolButtonAddPeriod_clicked();

	void on_toolButtonRemovePeriode_clicked();

	void on_tableWidgetPeriods_currentCellChanged(int currentRow, int currentColumn, int, int);

	void on_toolButtonBackward_clicked();

	void on_toolButtonForward_clicked();
	void on_tableWidgetPeriods_cellChanged(int row, int column);

	void on_tableWidgetPeriods_cellClicked(int row, int column);

	void on_checkBoxMonday_stateChanged(int arg1);

	void on_checkBoxTuesday_stateChanged(int arg1);

	void on_checkBoxHoliday_stateChanged(int arg1);

	void on_checkBoxWednesday_stateChanged(int arg1);

	void on_checkBoxThursday_stateChanged(int arg1);

	void on_checkBoxFriday_stateChanged(int arg1);

	void on_checkBoxSaturday_stateChanged(int arg1);

	void on_checkBoxSunday_stateChanged(int arg1);

private:

	/*! Update the period table. */
	void updatePeriodTable(const int &activeRow = 0);

	/*! Called when a new daily cycle has been selected,i.e. m_currentDailyCycleIndex has changed. */
	void selectDailyCycle();

	/*! Delete a daily cycle if no day type is checked. Can not delete daily cycle if only one daily cycle exists.
		Return true if a daily cycle was deleted.
	*/
	bool deleteDailyCycle();

	/*! If minimum one day type is enabled and checked this function returns true.*/
	bool isDayTypeChecked();

	void updateDayTypes(const NANDRAD::Schedule::ScheduledDayType &dt, bool checked);

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

	/*! Block StateChange of checkboxes. */
	bool								m_blockCheckBox=true;



};

//class SVDBScheduleDailyCycleEditWidget : public QWidget{
//	Q_OBJECT

//public:
//	explicit SVDBScheduleDailyCycleEditWidget(QWidget *parent = nullptr);
//	~SVDBScheduleDailyCycleEditWidget();

//};

#endif // SVDBScheduleEditWidget_H
