#include "SVMeasurementWidget.h"
#include "ui_SVMeasurementWidget.h"

#include "SVViewStateHandler.h"
#include "Vic3DMeasurementObject.h"

SVMeasurementWidget::SVMeasurementWidget(QWidget *parent) :
	QWidget(parent),
    m_ui(new Ui::SVMeasurementWidget)
{
    m_ui->setupUi(this);

    setWindowFlags(Qt::WindowStaysOnTopHint);

    SVViewStateHandler::instance().m_measurementWidget = this;
}

SVMeasurementWidget::~SVMeasurementWidget() {
    delete m_ui;
}

void SVMeasurementWidget::setMeasurementData(Vic3D::MeasurementObject &mo) {
    m_measurementObject = &mo;
}

void SVMeasurementWidget::setDistance() {
    Q_ASSERT(m_measurementObject != nullptr);

    QVector3D diff = m_measurementObject->m_startPoint - m_measurementObject->m_endPoint;

    m_ui->lineEditDistance->setText(QString::number(m_measurementObject->distance(),'f', 3) );
    m_ui->lineEditDistX->setText(QString::number(diff.x(),'f', 3) );
    m_ui->lineEditDistY->setText(QString::number(diff.y(),'f', 3) );
    m_ui->lineEditDistZ->setText(QString::number(diff.z(),'f', 3) );
}

void SVMeasurementWidget::setStartPoint() {
    Q_ASSERT(m_measurementObject != nullptr);
    m_ui->lineEditStartX->setText(QString::number(m_measurementObject->m_startPoint.x(),'f', 3) );
    m_ui->lineEditStartY->setText(QString::number(m_measurementObject->m_startPoint.y(),'f', 3) );
    m_ui->lineEditStartZ->setText(QString::number(m_measurementObject->m_startPoint.z(),'f', 3) );
}

void SVMeasurementWidget::setEndPoint() {
    Q_ASSERT(m_measurementObject != nullptr);
    m_ui->lineEditEndX->setText(QString::number(m_measurementObject->m_endPoint.x(),'f', 3) );
    m_ui->lineEditEndY->setText(QString::number(m_measurementObject->m_endPoint.y(),'f', 3) );
    m_ui->lineEditEndZ->setText(QString::number(m_measurementObject->m_endPoint.z(),'f', 3) );
}
