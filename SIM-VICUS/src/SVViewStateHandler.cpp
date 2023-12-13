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

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVColorLegend.h"
#include "SVMeasurementWidget.h"

SVViewStateHandler * SVViewStateHandler::m_self = nullptr;

SVViewStateHandler & SVViewStateHandler::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVViewStateHandler::instance]", "You must create an instance of "
		"SVViewStateHandler before accessing SVViewStateHandler::instance()!");
	return *m_self;
}

SVViewStateHandler::SVViewStateHandler(QObject * parent) :
	QObject(parent)
{
	// singleton check
	Q_ASSERT_X(m_self == nullptr, "[SVViewStateHandler::SVViewStateHandler]", "You must not create multiple instances of "
		"SVViewStateHandler!");
	m_self = this;
}


SVViewStateHandler::~SVViewStateHandler() {
	m_self = nullptr;
}


void SVViewStateHandler::setViewState(const SVViewState & newViewState) {
	m_viewState = newViewState;
	emit viewStateChanged();
}


void SVViewStateHandler::toggleTransparentWidgetsVisibility(SVMainWindow::MainViewMode mainView) {
	if (mainView == SVMainWindow::MV_GeometryView) {
		if (m_viewState.m_propertyWidgetMode == SVViewState::PM_ResultsProperties)
			m_colorLegend->setVisible(true);
		if (m_viewState.m_sceneOperationMode == SVViewState::OM_MeasureDistance)
			m_measurementWidget->setVisible(true);
	}
	else {
		m_measurementWidget->setVisible(false);
		m_colorLegend->setVisible(false);
	}
}


void SVViewStateHandler::refreshColors() {
	if (SVProjectHandler::instance().isValid())
		emit colorRefreshNeeded();
}
