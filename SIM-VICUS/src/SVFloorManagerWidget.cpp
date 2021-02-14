#include "SVFloorManagerWidget.h"
#include "ui_SVFloorManagerWidget.h"

SVFloorManagerWidget::SVFloorManagerWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVFloorManagerWidget)
{
	m_ui->setupUi(this);
	QStringList header;
	header << tr("Building") << tr("Floor/Level Name") << tr("Level Nr.") << tr("Height [m]");
	m_ui->treeWidget->setHeaderLabels(header);

	on_treeWidget_itemSelectionChanged();
}


SVFloorManagerWidget::~SVFloorManagerWidget() {
	delete m_ui;
}


void SVFloorManagerWidget::on_treeWidget_itemSelectionChanged() {
	// show/hide buttons depending on selection
	QList<QTreeWidgetItem*> sel = m_ui->treeWidget->selectedItems();

	// hide all buttons first
	m_ui->pushButtonAddLevel->setVisible(false);
	m_ui->pushButtonRemoveLevel->setVisible(false);
	m_ui->pushButtonRemoveBuilding->setVisible(false);


}
