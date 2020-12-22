#include "SVDBComponentEditWidget.h"
#include "ui_SVDBComponentEditWidget.h"

SVDBComponent::SVDBComponent(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBComponent)
{
	m_ui->setupUi(this);


}

SVDBComponent::~SVDBComponent()
{
	delete m_ui;
}
