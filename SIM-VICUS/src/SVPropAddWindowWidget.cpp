#include "SVPropAddWindowWidget.h"
#include "ui_SVPropAddWindowWidget.h"

#include <VICUS_Project.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"

SVPropAddWindowWidget::SVPropAddWindowWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropAddWindowWidget)
{
	m_ui->setupUi(this);

	SVViewStateHandler::instance().m_propAddWindowWidget = this;
}


SVPropAddWindowWidget::~SVPropAddWindowWidget() {
	delete m_ui;
}


void SVPropAddWindowWidget::setup() {
	// get selected objects - must have at least one surface selected
	const VICUS::Project & p = project();


	std::vector<const VICUS::Surface*> sel;
	bool haveAny = p.selectedSurfaces(sel, VICUS::Project::SG_Building);
	if (!haveAny) {
		m_ui->labelSelectSurfaces->show();
		m_ui->groupBoxWindows->setEnabled(false);
		// clear "new subsurfaces" object
		return;
	}
	else {
		m_ui->labelSelectSurfaces->hide();
		m_ui->groupBoxWindows->setEnabled(true);
	}
}
