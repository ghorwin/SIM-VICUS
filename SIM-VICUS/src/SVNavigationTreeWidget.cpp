#include "SVNavigationTreeWidget.h"
#include "ui_SVNavigationTreeWidget.h"

#include "SVStyle.h"

SVNavigationTreeWidget::SVNavigationTreeWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVNavigationTreeWidget)
{
	m_ui->setupUi(this);

	SVStyle::formatWidgetWithLayout(this);
}


SVNavigationTreeWidget::~SVNavigationTreeWidget() {
	delete m_ui;
}


void SVNavigationTreeWidget::onModified(int modificationType, void * data) {

}

