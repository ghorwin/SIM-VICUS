#include "SVDBDialogAddDependentElements.h"
#include "ui_SVDBDialogAddDependentElements.h"

#include <QtExt_Conversions.h>

#include <VICUS_AbstractDBElement.h>

#include <QMessageBox>

SVDBDialogAddDependentElements::SVDBDialogAddDependentElements(QWidget *parent):
	QDialog(parent),
	m_ui(new Ui::SVDBDialogAddDependentElements)
{
	m_ui->setupUi(this);
	setWindowTitle(tr("Add to user database"));
}


SVDBDialogAddDependentElements::~SVDBDialogAddDependentElements()
{
	delete m_ui;
}

void SVDBDialogAddDependentElements::setup(const QString &infoText, const std::set<VICUS::AbstractDBElement *> &elements)
{
	m_ui->labelText->setText(infoText);
	for (VICUS::AbstractDBElement *el: elements){
		QListWidgetItem *item = new QListWidgetItem;
		item->setText(QtExt::MultiLangString2QString(el->m_displayName));
		item->setFlags(!Qt::ItemIsEnabled | !Qt::ItemIsSelectable);
		m_ui->listWidget->addItem(item);
	}
}
