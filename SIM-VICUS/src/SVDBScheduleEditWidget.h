#ifndef SVDBScheduleEditWidget_H
#define SVDBScheduleEditWidget_H

#include <QWidget>


namespace VICUS {
class Schedule;
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

private:
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
};

#endif // SVDBScheduleEditWidget_H
