#include "SVMeasurementWidget.h"
#include "ui_SVMeasurementWidget.h"

#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "SVStyle.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"

#include "Vic3DMeasurementObject.h"
#include "Vic3DCoordinateSystemObject.h"

#include <IBKMK_3DCalculations.h>

#include <SVConversions.h>

#include <QClipboard>
#include <QTextStream>
#include <QColorDialog>
#include <QDebug>
#include <QSizePolicy>

SVMeasurementWidget::SVMeasurementWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVMeasurementWidget)
{
	m_ui->setupUi(this);

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);

	setWindowOpacity(0.8);

	SVViewStateHandler::instance().m_measurementWidget = this;
}


SVMeasurementWidget::~SVMeasurementWidget() {
	delete m_ui;
}


void SVMeasurementWidget::reset() {
	m_ui->lineEditDistX->clear();
	m_ui->lineEditDistY->clear();
	m_ui->lineEditDistZ->clear();
	m_ui->lineEditDistance->clear();
	m_ui->lineEditEndX->clear();
	m_ui->lineEditEndY->clear();
	m_ui->lineEditEndZ->clear();
	m_ui->lineEditStartX->clear();
	m_ui->lineEditStartY->clear();
	m_ui->lineEditStartZ->clear();
}

void SVMeasurementWidget::setPosition(const QPoint &position) {
	move(position.x()-width(), position.y()-height() );
}

void SVMeasurementWidget::showStartPoint(const QVector3D &sp) {
	m_startPoint = sp;
	m_ui->lineEditStartX->setText(QString("%L1").arg((double)sp.x(), 0, 'f', 3) );
	m_ui->lineEditStartY->setText(QString("%L1").arg((double)sp.y(), 0, 'f', 3) );
	m_ui->lineEditStartZ->setText(QString("%L1").arg((double)sp.z(), 0, 'f', 3) );
}

void SVMeasurementWidget::showEndPoint(const QVector3D &ep) {
	m_endPoint = ep;
	m_ui->lineEditEndX->setText(QString("%L1").arg((double)ep.x(), 0, 'f', 3) );
	m_ui->lineEditEndY->setText(QString("%L1").arg((double)ep.y(), 0, 'f', 3) );
	m_ui->lineEditEndZ->setText(QString("%L1").arg((double)ep.z(), 0, 'f', 3) );

	showMeasurement();
}

void SVMeasurementWidget::showMeasurement() {
	QVector3D diff;
	if(!m_ui->checkBoxLocalMeasurement->isChecked())
		diff = m_endPoint - m_startPoint;
	else {
		IBKMK::Vector3D vLocal, point;

		IBKMK::lineToPointDistance(QVector2IBKVector(m_startPoint), QVector2IBKVector(m_localXAxis), QVector2IBKVector(m_endPoint), vLocal.m_x, point);
		IBKMK::lineToPointDistance(QVector2IBKVector(m_startPoint), QVector2IBKVector(m_localYAxis), QVector2IBKVector(m_endPoint), vLocal.m_y, point);
		IBKMK::lineToPointDistance(QVector2IBKVector(m_startPoint), QVector2IBKVector(m_localZAxis), QVector2IBKVector(m_endPoint), vLocal.m_z, point);

		diff = IBKVector2QVector(vLocal);
	}

	m_ui->lineEditDistance->setText(QString("%L1").arg((double)diff.length(), 0, 'f', 3) );
	m_ui->lineEditDistX->setText(QString("%L1").arg((double)diff.x(), 0, 'f', 3) );
	m_ui->lineEditDistY->setText(QString("%L1").arg((double)diff.y(), 0, 'f', 3) );
	m_ui->lineEditDistZ->setText(QString("%L1").arg((double)diff.z(), 0, 'f', 3) );
}

void SVMeasurementWidget::setLocalAxes(const QVector3D & xAxis, const QVector3D & yAxis, const QVector3D & zAxis) {
	m_localXAxis = xAxis;
	m_localYAxis = yAxis;
	m_localZAxis = zAxis;
}

void SVMeasurementWidget::on_pushButtonCopyInformation_clicked() {
	// Note: this text is always copied using english locale settings, for easy of parsing in external applications
	QString clipText = QString("Start point:\t%1\t%2\t%3\nEnd point:\t%4\t%5\t%6\nDistance:\t%7\n\t\tdX:%7\tdY:%8\tdZ:%9")
			.arg(m_ui->lineEditStartX->text() )
			.arg(m_ui->lineEditStartY->text() )
			.arg(m_ui->lineEditStartZ->text() )
			.arg(m_ui->lineEditEndX->text() )
			.arg(m_ui->lineEditEndY->text() )
			.arg(m_ui->lineEditEndZ->text() )
			.arg(m_ui->lineEditDistance->text() )
			.arg(m_ui->lineEditDistX->text())
			.arg(m_ui->lineEditDistY->text())
			.arg(m_ui->lineEditDistZ->text());

	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->setText(clipText);
}


void SVMeasurementWidget::on_pushButtonColor_colorChanged() {
	m_color = m_ui->pushButtonColor->color();
	SVViewStateHandler::instance().m_geometryView->refreshSceneView();
	SVViewStateHandler::instance().m_coordinateSystemObject->setOrbColor(m_color);
}

void SVMeasurementWidget::on_checkBoxLocalMeasurement_toggled(bool /*on*/) {
	showMeasurement();
}

