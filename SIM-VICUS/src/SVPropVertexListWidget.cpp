#include "SVPropVertexListWidget.h"
#include "ui_SVPropVertexListWidget.h"

#include <IBKMK_Vector3D.h>

#include "SVViewStateHandler.h"
#include "Vic3DNewPolygonObject.h"

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

	m_ui->pushButtonDeleteLast->setEnabled(true);

	m_ui->pushButtonFinish->setEnabled(SVViewStateHandler::instance().m_newPolygonObject->canComplete());
}


void SVPropVertexListWidget::onNewVertexListStart() {
	// clear table widget and disable "delete" and "finish" buttons
	m_ui->tableWidgetVertexes->setRowCount(0);
	m_ui->pushButtonFinish->setEnabled(false);
	m_ui->pushButtonDeleteLast->setEnabled(false);
	m_ui->pushButtonDeleteSelected->setEnabled(false);
}


void SVPropVertexListWidget::on_pushButtonDeleteLast_clicked() {
	int rows = m_ui->tableWidgetVertexes->rowCount();
	Q_ASSERT(rows > 0);
	// remove last vertex from polygon
	Vic3D::NewPolygonObject * po = SVViewStateHandler::instance().m_newPolygonObject;
	po->removeVertex(rows-1);

	m_ui->pushButtonDeleteLast->setEnabled(rows > 1);
	m_ui->tableWidgetVertexes->setRowCount(rows-1);
	m_ui->pushButtonFinish->setEnabled(SVViewStateHandler::instance().m_newPolygonObject->canComplete());
}


void SVPropVertexListWidget::on_tableWidgetVertexes_itemSelectionChanged() {
	m_ui->pushButtonDeleteSelected->setEnabled( m_ui->tableWidgetVertexes->currentRow() != -1);
}


void SVPropVertexListWidget::on_pushButtonCancel_clicked() {
	// reset new polygon object, so that it won't be drawn anylonger
	SVViewStateHandler::instance().m_newPolygonObject->clear();
	// signal, that we are no longer in "add vertex" mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	vs.m_propertyWidgetMode = SVViewState::PM_ADD_GEOMETRY;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// also prepare the table widget for next time use
	onNewVertexListStart();
}


void SVPropVertexListWidget::on_pushButtonDeleteSelected_clicked() {
	int rows = m_ui->tableWidgetVertexes->rowCount();
	Q_ASSERT(rows > 0);
	int currentRow = m_ui->tableWidgetVertexes->currentRow();
	Q_ASSERT(currentRow != -1);
	// remove selected vertex from polygon
	Vic3D::NewPolygonObject * po = SVViewStateHandler::instance().m_newPolygonObject;
	po->removeVertex((unsigned int)currentRow);

	m_ui->pushButtonDeleteLast->setEnabled(rows > 1);
	// now remove selected row from table widget
	m_ui->tableWidgetVertexes->removeRow(currentRow);
	m_ui->pushButtonFinish->setEnabled(SVViewStateHandler::instance().m_newPolygonObject->canComplete());
}
