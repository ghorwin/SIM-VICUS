#include "SVLocalCoordinateView.h"
#include "SVViewStateHandler.h"

#include "ui_SVLocalCoordinateView.h"

SVLocalCoordinateView::SVLocalCoordinateView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVLocalCoordinateView)
{
	m_ui->setupUi(this);
	m_ui->horizontalLayout->setMargin(0);

	// make us known to the world
	SVViewStateHandler::instance().m_localCoordinateViewWidget = this;
}


SVLocalCoordinateView::~SVLocalCoordinateView() {
	delete m_ui;
}


void SVLocalCoordinateView::setCoordinates(const Vic3D::Transform3D &t) {

	// is being call from local coordinate system object, whenever this has changed location (regardless of
	// its own visibility)
	m_ui->lineEditXValue->setText( QString("%L1").arg( (double)t.translation().x(), 0, 'f', 3 ) );
	m_ui->lineEditYValue->setText( QString("%L1").arg( (double)t.translation().y(), 0, 'f', 3 ) );
	m_ui->lineEditZValue->setText( QString("%L1").arg( (double)t.translation().z(), 0, 'f', 3 ) );
}

