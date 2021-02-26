#ifndef SVDBSCHEDULEADDDIALOG_H
#define SVDBSCHEDULEADDDIALOG_H

#include <QDialog>

namespace VICUS {
class Schedule;
}

namespace Ui {
class SVDBScheduleAddDialog;
}

class SVDBScheduleAddDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVDBScheduleAddDialog(QWidget *parent = nullptr);
	~SVDBScheduleAddDialog();

	static bool requestScheduleData(const QString &title, VICUS::Schedule &sched);


	private:
		Ui::SVDBScheduleAddDialog *m_ui;
};

#endif // SVDBSCHEDULEADDDIALOG_H
