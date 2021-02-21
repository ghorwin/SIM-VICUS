#include "SVDBScheduleDailyCycleEditWidget.h"
#include "ui_SVDBScheduleDailyCycleEditWidget.h"

SVDBScheduleDailyCycleEditWidget::SVDBScheduleDailyCycleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBScheduleDailyCycleEditWidget)
{
	m_ui->setupUi(this);


	//add header to day cycle table
	m_ui->tableWidgetDayCycle->setColumnCount(2);
	m_ui->tableWidgetDayCycle->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Time")));
	m_ui->tableWidgetDayCycle->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Value")));

	//set all table items in day cycle
	m_ui->tableWidgetDayCycle->setRowCount(24);
	for(unsigned int i=0; i<24; ++i){
		QString time = i < 9 ? "0" + QString::number(i) : QString::number(i);
		time += ":00 - ";
		time += i+1 < 9 ? "0" + QString::number(i+1) : QString::number(i+1);
		time += ":00";

		m_ui->tableWidgetDayCycle->setItem(i,0, new QTableWidgetItem(time));
		m_ui->tableWidgetDayCycle->setItem(i,1, new QTableWidgetItem("---"));
	}

}

SVDBScheduleDailyCycleEditWidget::~SVDBScheduleDailyCycleEditWidget()
{
	delete m_ui;
}


