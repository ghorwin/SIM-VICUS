#include "SVPropVertexListWidget.h"
#include "ui_SVPropVertexListWidget.h"

#include <IBKMK_Vector3D.h>

SVPropVertexListWidget::SVPropVertexListWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropVertexListWidget)
{
	m_ui->setupUi(this);
	onNewVertexListStart();
}


SVPropVertexListWidget::~SVPropVertexListWidget() {
	delete m_ui;
}


void SVPropVertexListWidget::addVertex(const IBKMK::Vector3D & p) {
	int row = m_ui->tableWidgetVertexes->rowCount();
	m_ui->tableWidgetVertexes->setRowCount(row + 1);
	QTableWidgetItem * item = new QTableWidgetItem(QString("%1").arg(row+1));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,0,item);
	item = new QTableWidgetItem(QString("%L1,%L2,%L3").arg(p.m_x).arg(p.m_y).arg(p.m_z));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,1,item);

	m_ui->pushButtonDeleteLast->setEnabled(false);

	// TODO : finish can only be enabled, once view state signals "complete"
	m_ui->pushButtonFinish->setEnabled(false);
}


void SVPropVertexListWidget::onNewVertexListStart() {
	// clear table widget and disable "delete" and "finish" buttons
	m_ui->tableWidgetVertexes->clearContents();
	m_ui->pushButtonFinish->setEnabled(false);
	m_ui->pushButtonDeleteLast->setEnabled(false);
	m_ui->pushButtonDeleteSelected->setEnabled(false);
}
