#include "SVLocalCoordinateView.h"
#include "SVViewStateHandler.h"

#include <IBK_physics.h>

#include <QtExt_Conversions.h>

#include "ui_SVLocalCoordinateView.h"


SVLocalCoordinateView::SVLocalCoordinateView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVLocalCoordinateView)
{
	m_ui->setupUi(this);
	m_ui->toolButton->isCheckable();
	QPixmap mypix (":/gfx/actions/icons8-coordinate-system-32.png");
//	m_ui->labelIcon->setPixmap( mypix.scaledToHeight(32) );

	SVViewStateHandler::instance().m_localCoordinateViewWidget = this;
}

SVLocalCoordinateView::~SVLocalCoordinateView()
{
	delete m_ui;
}


void SVLocalCoordinateView::setCoordinates(const Vic3D::Transform3D &t)
{
		// is being call from local coordinate system object, whenever this has changed location (regardless of
		// its own visibility)
		m_ui->lineEditXValue->setText( QString("%L1").arg( t.translation().x(), 4, 'f' ) );
		m_ui->lineEditYValue->setText( QString("%L1").arg( t.translation().y(), 4, 'f' ) );
		m_ui->lineEditZValue->setText( QString("%L1").arg( t.translation().z(), 4, 'f' ) );
}

void SVLocalCoordinateView::setOrientation(const QVector3D &x, const QVector3D &y, const QVector3D &z)
{	QVector3D t;

	m_xAxis = x;
	m_yAxis = y;
	m_zAxis = z;

	if ( m_ui->comboBoxAxis->currentIndex() == 0 ) //x-Axis
		 t = x;
	else if ( m_ui->comboBoxAxis->currentIndex() == 1 ) // y-Axis
		t = y;
	else
		t = z;

	m_ui->lineEditInclination->setText( QString("%L1").arg( std::acos(t.z())/IBK::DEG2RAD, 3, 'f' ) );

	// positive y Richtung = Norden = Orientation 0°
	// positive x Richtung = Osten = Orientation 90°

	double orientation = std::atan2(t.x(), ( t.y() == 0 ? 1E-8 : t.y() ) ) /IBK::DEG2RAD ;
	m_ui->lineEditOrientation->setText( QString("%L1").arg( orientation < 0 ? ( orientation + 360 ) : orientation , 3, 'f') );
}

void SVLocalCoordinateView::on_comboBoxAxis_currentIndexChanged(int index)
{
	setOrientation(m_xAxis, m_yAxis, m_zAxis);
}

void SVLocalCoordinateView::on_toolButton_toggled(bool positionOn)
{
	if ( positionOn ) {
		m_ui->labelX->show();
		m_ui->labelY->show();
		m_ui->labelZ->show();
		m_ui->lineEditXValue->show();
		m_ui->lineEditYValue->show();
		m_ui->lineEditZValue->show();
		} else {
		m_ui->labelX->hide();
		m_ui->labelY->hide();
		m_ui->labelZ->hide();
		m_ui->lineEditXValue->hide();
		m_ui->lineEditYValue->hide();
		m_ui->lineEditZValue->hide();
	}
}

void SVLocalCoordinateView::on_toolButtonOrientation_toggled(bool coordinationOn)
{
	if ( coordinationOn ) {
		m_ui->comboBoxAxis->show();
		m_ui->labelRotateInclinationAbs->show();
		m_ui->labelRotateOrientationAbs->show();
		m_ui->lineEditInclination->show();
		m_ui->lineEditOrientation->show();
//		m_ui->lineCoordinate->show();
	} else {
		m_ui->comboBoxAxis->hide();
		m_ui->labelRotateInclinationAbs->hide();
		m_ui->labelRotateOrientationAbs->hide();
		m_ui->lineEditInclination->hide();
		m_ui->lineEditOrientation->hide();
//		m_ui->lineCoordinate->hide();
	}
}
