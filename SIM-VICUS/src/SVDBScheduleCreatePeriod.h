#ifndef SVDBScheduleCreatePeriodH
#define SVDBScheduleCreatePeriodH

#include <QWidget>

namespace Ui {
class SVDBScheduleCreatePeriod;
}

class SVDBScheduleCreatePeriod : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBScheduleCreatePeriod(QWidget *parent = nullptr);
	~SVDBScheduleCreatePeriod();



private slots:

	void on_toolButtonApply_clicked();

	void on_toolButtonRemove_clicked();

signals:
	void dayChanged(int startDay);

private:
	Ui::SVDBScheduleCreatePeriod *m_ui;


};

#endif // SVDBScheduleCreatePeriodH
