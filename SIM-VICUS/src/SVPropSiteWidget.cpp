/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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
			Q_ASSERT(project().m_viewSettings.m_gridPlanes.size()>0);
			m_ui->lineEditMaxDimensions->setText(QString("%L1").arg(project().m_viewSettings.m_gridPlanes[0].m_width));
			m_ui->lineEditGridLineSpacing->setText(QString("%L1").arg(project().m_viewSettings.m_gridPlanes[0].m_spacing));
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
		std::vector<VICUS::GridPlane> gridPlanes(project().m_viewSettings.m_gridPlanes);
		gridPlanes[0].m_width = d;
		SVUndoModifySiteData * undo = new SVUndoModifySiteData(tr("Grid property changed"),
																 gridPlanes,
																 project().m_viewSettings.m_farDistance);
		undo->push();
	}
}


void SVPropSiteWidget::on_lineEditGridLineSpacing_editingFinished() {
	// try to get a valid number of the line edit
	bool ok;
	double d = QLocale().toDouble(m_ui->lineEditGridLineSpacing->text(), &ok);
	if (ok && d > 0.1) {
		std::vector<VICUS::GridPlane> gridPlanes(project().m_viewSettings.m_gridPlanes);
		gridPlanes[0].m_spacing = d;
		SVUndoModifySiteData * undo = new SVUndoModifySiteData(tr("Grid property changed"),
															   gridPlanes,
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
															   project().m_viewSettings.m_gridPlanes,
															   d);
		undo->push();
	}

}
