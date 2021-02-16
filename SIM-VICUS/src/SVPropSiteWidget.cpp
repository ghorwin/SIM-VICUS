#include "SVPropSiteWidget.h"
#include "ui_SVPropSiteWidget.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVUndoModifySiteData.h"

SVPropSiteWidget::SVPropSiteWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropSiteWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropSiteWidget::onModified);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);
}


SVPropSiteWidget::~SVPropSiteWidget() {
	delete m_ui;
}


void SVPropSiteWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	switch (modificationType) {
		case SVProjectHandler::AllModified :
		case SVProjectHandler::GridModified :
			// transfer data to user interface elements
			m_ui->lineEditMaxDimensions->setText(QString("%L1").arg(project().m_viewSettings.m_gridWidth));
			m_ui->lineEditGridLineSpacing->setText(QString("%L1").arg(project().m_viewSettings.m_gridSpacing));
			m_ui->lineEditViewDepth->setText(QString("%L1").arg(project().m_viewSettings.m_farDistance));
		break;

		default:
			return;
	}
}


void SVPropSiteWidget::on_lineEditMaxDimensions_editingFinished() {
	// try to get a valid number of the line edit
	bool ok;
	double d = QLocale().toDouble(m_ui->lineEditMaxDimensions->text(), &ok);
	if (ok && d > 10) {
		SVUndoModifySiteData * undo = new SVUndoModifySiteData(tr("Grid property changed"),
																 d,
																 project().m_viewSettings.m_gridSpacing,
																 project().m_viewSettings.m_farDistance);
		undo->push();
	}
}


void SVPropSiteWidget::on_lineEditGridLineSpacing_editingFinished() {
	// try to get a valid number of the line edit
	bool ok;
	double d = QLocale().toDouble(m_ui->lineEditGridLineSpacing->text(), &ok);
	if (ok && d > 0.1) {
		SVUndoModifySiteData * undo = new SVUndoModifySiteData(tr("Grid property changed"),
																 project().m_viewSettings.m_gridWidth,
																 d,
																 project().m_viewSettings.m_farDistance);
		undo->push();
	}
}


void SVPropSiteWidget::on_lineEditViewDepth_editingFinished() {
	// try to get a valid number of the line edit
	bool ok;
	double d = QLocale().toDouble(m_ui->lineEditViewDepth->text(), &ok);
	if (ok && d > 100) {
		SVUndoModifySiteData * undo = new SVUndoModifySiteData(tr("Grid property changed"),
																 project().m_viewSettings.m_gridWidth,
																 project().m_viewSettings.m_gridSpacing,
																 d);
		undo->push();
	}

}
