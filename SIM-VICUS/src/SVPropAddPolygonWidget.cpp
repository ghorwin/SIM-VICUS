#include "SVPropAddPolygonWidget.h"
#include "ui_SVPropAddPolygonWidget.h"

#include <QKeyEvent>
#include <QDebug>

SVPropAddPolygonWidget::SVPropAddPolygonWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropAddPolygonWidget)
{
	m_ui->setupUi(this);
}


SVPropAddPolygonWidget::~SVPropAddPolygonWidget() {
	delete m_ui;
}


void SVPropAddPolygonWidget::keyPressEvent(QKeyEvent * event) {

	// listen to enter-presses when focus is on table widget
	if (!m_ui->tableWidgetVertexes->hasFocus()) {
		event->ignore();
		return;
	}
	if (event->key() == Qt::Key_Delete) {
		qDebug() << "Deleting vertex";
	}

}


void SVPropAddPolygonWidget::on_toolButtonPlane_clicked() {
	// signal change into "add polygon" mode
}


void SVPropAddPolygonWidget::on_toolButtonBox_clicked() {
	// signal change into "add box" mode
}


void SVPropAddPolygonWidget::on_pushButtonAdd_clicked() {
	// signal completion of "Add polygon" operation
}


void SVPropAddPolygonWidget::on_pushButtonCancel_clicked() {
	// signal abort of "Add polygon" operation
}
