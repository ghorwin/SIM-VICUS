#include "SVSnapOptionsDialog.h"
#include "ui_SVSnapOptionsDialog.h"

#include "SVViewStateHandler.h"
#include "SVGeometryView.h"

SVSnapOptionsDialog::SVSnapOptionsDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSnapOptionsDialog)
{
	m_ui->setupUi(this);

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);

	layout()->setContentsMargins(2,2,2,2);
	layout()->setSpacing(1);
	m_ui->widgetSnapDistance->setContentsMargins(0,0,0,0);
	m_ui->widgetSnapDistance->layout()->setSpacing(1);
	adjustSize();
	setWindowOpacity(0.75);

	m_ui->lineEditSnapDistance->setup(1e-3, 10, tr("Snapping distance threshold in m"));

	SVViewStateHandler::instance().m_snapOptionsDialog = this;
}

SVSnapOptionsDialog::~SVSnapOptionsDialog() {
	delete m_ui;
}

void SVSnapOptionsDialog::updateUi() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	m_ui->checkBoxGridPlane->setChecked(vs.m_snapOptionMask & SVViewState::Snap_GridPlane);
	m_ui->checkBoxObjectCenter->setChecked(vs.m_snapOptionMask & SVViewState::Snap_ObjectCenter);
	m_ui->checkBoxObjectEdge->setChecked(vs.m_snapOptionMask & SVViewState::Snap_ObjectEdgeCenter);
	m_ui->checkBoxObjectVertex->setChecked(vs.m_snapOptionMask & SVViewState::Snap_ObjectVertex);
	m_ui->checkBoxDrawing->setChecked(vs.m_snapOptionMask & SVViewState::Snap_Drawings);
	m_ui->checkBoxDrawingLine->setChecked(vs.m_snapOptionMask & SVViewState::Snap_DrawingLines);
	m_ui->lineEditSnapDistance->setValue((double)vs.m_snapDistance);
}

void SVSnapOptionsDialog::setPosition(const QPoint & position) {
	move(position.x(), position.y()-height() );
}

void SVSnapOptionsDialog::on_checkBoxGridPlane_clicked(bool checked) {
	if (checked)
		SVViewStateHandler::instance().addSnapOption(SVViewState::Snap_GridPlane);
	else
		SVViewStateHandler::instance().removeSnapOption(SVViewState::Snap_GridPlane);
}

void SVSnapOptionsDialog::on_checkBoxDrawing_clicked(bool checked) {
	if (checked)
		SVViewStateHandler::instance().addSnapOption(SVViewState::Snap_Drawings);
	else
		SVViewStateHandler::instance().removeSnapOption(SVViewState::Snap_Drawings);
}

void SVSnapOptionsDialog::on_checkBoxDrawingLine_clicked(bool checked){
	if (checked)
		SVViewStateHandler::instance().addSnapOption(SVViewState::Snap_DrawingLines);
	else
		SVViewStateHandler::instance().removeSnapOption(SVViewState::Snap_DrawingLines);
}

void SVSnapOptionsDialog::on_checkBoxObjectVertex_clicked(bool checked) {
	if (checked)
		SVViewStateHandler::instance().addSnapOption(SVViewState::Snap_ObjectVertex);
	else
		SVViewStateHandler::instance().removeSnapOption(SVViewState::Snap_ObjectVertex);
}

void SVSnapOptionsDialog::on_checkBoxObjectCenter_clicked(bool checked) {
	if (checked)
		SVViewStateHandler::instance().addSnapOption(SVViewState::Snap_ObjectCenter);
	else
		SVViewStateHandler::instance().removeSnapOption(SVViewState::Snap_ObjectCenter);
}

void SVSnapOptionsDialog::on_checkBoxObjectEdge_clicked(bool checked) {
	if (checked)
		SVViewStateHandler::instance().addSnapOption(SVViewState::Snap_ObjectEdgeCenter);
	else
		SVViewStateHandler::instance().removeSnapOption(SVViewState::Snap_ObjectEdgeCenter);
}

void SVSnapOptionsDialog::on_toolButton_clicked(){
	setExpanded(!m_expanded);
}

void SVSnapOptionsDialog::on_lineEditSnapDistance_editingFinished() {
	if (m_ui->lineEditSnapDistance->isValid())
		SVViewStateHandler::instance().setSnapDistance((float)m_ui->lineEditSnapDistance->value());
}

void SVSnapOptionsDialog::setExpanded(bool expanded) {
	m_expanded = expanded;
	int w = width();
	m_ui->checkBoxGridPlane->setVisible(expanded);
	m_ui->checkBoxObjectCenter->setVisible(expanded);
	m_ui->checkBoxObjectEdge->setVisible(expanded);
	m_ui->checkBoxObjectVertex->setVisible(expanded);
	m_ui->checkBoxDrawing->setVisible(expanded);
	m_ui->checkBoxDrawingLine->setVisible(expanded);
	m_ui->widgetSnapDistance->setVisible(expanded);
	if (expanded) {
		m_ui->toolButton->setArrowType(Qt::ArrowType::DownArrow);
		adjustSize();
	}
	else {
		m_ui->toolButton->setArrowType(Qt::ArrowType::UpArrow);
		adjustSize();
		resize(w, height());
	}
	SVViewStateHandler::instance().m_geometryView->moveTransparentSceneWidgets();
}



