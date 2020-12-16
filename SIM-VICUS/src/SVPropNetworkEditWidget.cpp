#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"

SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);
}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::updateUi() {
	// get currently selected network
	unsigned int selectedNetworkUniqueId = SVViewStateHandler::instance().m_navigationTreeWidget->selectedNodeID();

	// TODO : Hauke
}
