#include "SVDBSubNetworkEditWidget.h"
#include "ui_SVDBSubNetworkEditWidget.h"

SVDBSubNetworkEditWidget::SVDBSubNetworkEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBSubNetworkEditWidget)
{
	m_ui->setupUi(this);
}

SVDBSubNetworkEditWidget::~SVDBSubNetworkEditWidget()
{
	delete m_ui;
}

void SVDBSubNetworkEditWidget::setup(SVDatabase *db, SVAbstractDatabaseTableModel *dbModel)
{

}

void SVDBSubNetworkEditWidget::updateInput(int id)
{

}

void SVDBSubNetworkEditWidget::modelModify()
{

}
