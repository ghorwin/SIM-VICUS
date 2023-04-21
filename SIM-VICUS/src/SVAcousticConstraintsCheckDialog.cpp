#include "SVAcousticConstraintsCheckDialog.h"
#include "ui_SVAcousticConstraintsCheckDialog.h"

#include "SVStyle.h"

SVAcousticConstraintsCheckDialog::SVAcousticConstraintsCheckDialog(QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::SVAcousticConstraintsCheckDialog)
{
	m_ui->setupUi(this);

	m_ui->tableWidget->setColumnCount(6);
	m_ui->tableWidget->setHorizontalHeaderLabels(QStringList() << QString() << tr("Surface A")<< tr("Surface B")<< tr("is valid")<< tr("actual Values")<< tr("expected limits"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidget);
	m_ui->tableWidget->setSortingEnabled(false);
	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidget->horizontalHeader()->resizeSection(0,20);
	//m_ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

SVAcousticConstraintsCheckDialog::~SVAcousticConstraintsCheckDialog()
{
	delete m_ui;
}


bool SVAcousticConstraintsCheckDialog::edit(){
	int res = exec();
	return res;
}
