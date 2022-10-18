#include "SVDBDialogAddDependentElements.h"
#include "ui_SVDBDialogAddDependentElements.h"

#include <SVConversions.h>
#include <VICUS_AbstractDBElement.h>

SVDBDialogAddDependentElements::SVDBDialogAddDependentElements(QWidget *parent):
	QDialog(parent),
	m_ui(new Ui::SVDBDialogAddDependentElements)
{
	m_ui->setupUi(this);
}


SVDBDialogAddDependentElements::~SVDBDialogAddDependentElements() {
	delete m_ui;
}


void SVDBDialogAddDependentElements::setup(const std::set<VICUS::AbstractDBElement *> &elements) {
	for (const VICUS::AbstractDBElement *el: elements){
		QListWidgetItem *item = new QListWidgetItem;
		item->setText(QtExt::MultiLangString2QString(el->m_displayName));
		item->setFlags(item->flags() &  ~Qt::ItemIsSelectable);
		m_ui->listWidget->addItem(item);
	}
}
