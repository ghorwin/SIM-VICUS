#include "SVPropVertexListWidget.h"
#include "ui_SVPropVertexListWidget.h"

SVPropVertexListWidget::SVPropVertexListWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropVertexListWidget)
{
	m_ui->setupUi(this);
}


SVPropVertexListWidget::~SVPropVertexListWidget() {
	delete m_ui;
}
