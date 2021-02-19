#include "SVDBScheduleCreatePeriod.h"
#include "ui_SVDBScheduleCreatePeriod.h"

SVDBScheduleCreatePeriod::SVDBScheduleCreatePeriod(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleCreatePeriod)
{
	m_ui->setupUi(this);
}

SVDBScheduleCreatePeriod::~SVDBScheduleCreatePeriod()
{
	delete m_ui;
}

void SVDBScheduleCreatePeriod::on_toolButtonApply_clicked(){
	emit dayChanged(m_ui->dateEdit->date().dayOfYear()-1);
	close();
}

void SVDBScheduleCreatePeriod::on_toolButtonRemove_clicked(){
	close();
}
