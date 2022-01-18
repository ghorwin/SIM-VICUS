/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_ViewBase.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QShortcut>
#include <QKeySequence>
#include <QSettings>
#include <QFrame>
#include <QSplitter>

#include "QtExt_Constants.h"
#include "QtExt_PushButton.h"
#include "QtExt_PanelButton.h"


namespace QtExt {


ViewBase::ViewBase(QWidget * parent, const QKeySequence keySequence) :
	QWidget(parent)
{
	m_viewButton = new PushButton(this);
	m_shortcut = new QShortcut( keySequence, parent, 0, 0, Qt::WindowShortcut );
}


ViewBase::~ViewBase(){
  //	qDebug() << "ViewBase::~ViewBase";
}


void ViewBase::initPanels(QWidget * centerPanel, QWidget * westPanel,
						  QWidget * eastPanel, QWidget * southPanel,
						  bool spHasEastButton, bool spHasWestButton)
{
	// center panel must exist
	Q_ASSERT(centerPanel != NULL);

	m_centerPanel = centerPanel;
	m_westPanel = westPanel;
	m_eastPanel = eastPanel;
	m_southPanel = southPanel;
	m_widgetTopPart = new QWidget(this);
	m_widgetBottomPart = new QWidget(this);

	// main layout of a view is a vertical layout holding a m_verticalSplitter with
	// m_widgetTopPart and m_widgetBottomPart
	QVBoxLayout * vlay = new QVBoxLayout();

	// *** Setup Top part (m_widgetTopPart) ***

	// west, center and east panels and their buttons are aligned in a
	// horizontal box layout
	QHBoxLayout * hlay = new QHBoxLayout();
	m_widgetTopPart->setLayout(hlay);
	m_horizontalSplitter = new QSplitter(this);

	// west panel button
	m_wpButton = new PanelButton( this, PanelButton::DD_WEST );
	hlay->addWidget(m_wpButton);
	// west panel widget
	if (m_westPanel	!= NULL) {
		m_horizontalSplitter->addWidget(m_westPanel);
		m_lastWestPanelSize = m_westPanel->sizeHint().width();
		connect(m_wpButton, SIGNAL(clicked()), this, SLOT(on_westPanelButton_Clicked()));
	}
	else {
		m_wpButton->setEnabled(false);
	}

	// center widget
	m_horizontalSplitter->addWidget(m_centerPanel);
	m_horizontalSplitter->setChildrenCollapsible(false);

	m_epButton = new PanelButton( this, PanelButton::DD_EAST );
	// east panel widget
	if (m_eastPanel	!= NULL) {
		m_horizontalSplitter->addWidget(m_eastPanel);
		m_lastEastPanelSize = m_eastPanel->sizeHint().width();
		connect(m_epButton, SIGNAL(clicked()), this, SLOT(on_eastPanelButton_Clicked()));
	}
	else {
		m_epButton->setEnabled(false);
	}

	// add splitter to hlayout
	hlay->addWidget(m_horizontalSplitter);

	// east panel button
	hlay->addWidget(m_epButton);

	// *** Setup Bottom part (m_widgetBottomPart) ***

	// South panel button always exists
	m_spButton = new PanelButton( this, PanelButton::DD_SOUTH );
	m_spButton->setShortcut(QKeySequence());

	if (southPanel != NULL) {
		m_spEastButton = new PanelButton( this, PanelButton::DD_EAST );
		m_spEastButton->setShortcut(QKeySequence());
		m_spEastButton->setMaximumWidth(PANEL_BUTTON_SIZE);
//		m_spEastButton->setMaximumHeight(SOUTH_PANEL_HEIGHT);
		m_spEastButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
//		m_spEastButton->setFlat(true);

		m_spWestButton = new PanelButton( this, PanelButton::DD_WEST );
		m_spWestButton->setShortcut(QKeySequence());
		m_spWestButton->setMaximumWidth(PANEL_BUTTON_SIZE);
//		m_spWestButton->setMaximumHeight(SOUTH_PANEL_HEIGHT);
		m_spWestButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
//		m_spWestButton->setFlat(true);

		if (spHasEastButton) {
			connect(m_spEastButton, SIGNAL(clicked()), this, SLOT(on_spEastButton_Clicked()));
		}
		else {
			m_spEastButton->setEnabled(false);
		}
		if (spHasWestButton) {
			connect(m_spWestButton, SIGNAL(clicked()), this, SLOT(on_spWestButton_Clicked()));
		}
		else {
			m_spWestButton->setEnabled(false);
		}
		// \todo Remove once buttons have a meaning


		QHBoxLayout * hlay_south = new QHBoxLayout();
		QVBoxLayout * vlay_south = new QVBoxLayout();
		m_widgetBottomPart->setSizePolicy(southPanel->sizePolicy());
		m_widgetBottomPart->resize(southPanel->width(), QtExt::SOUTH_PANEL_HEIGHT);
		m_widgetBottomPart->setLayout(vlay_south);
		hlay_south->addWidget(m_spWestButton);
		hlay_south->addWidget(southPanel);
		hlay_south->addWidget(m_spEastButton);

		hlay_south->setContentsMargins(0, 0, 0, 0);
		hlay_south->setMargin(0);
		hlay_south->setSpacing(1);

		// add separator for orientation
		QFrame * line = new QFrame;
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		vlay_south->addWidget(line);
		vlay_south->addLayout(hlay_south);
		vlay_south->setMargin(0);


		m_verticalSplitter = new QSplitter(this);
		m_verticalSplitter->setChildrenCollapsible(false);
		m_verticalSplitter->setOrientation(Qt::Vertical);
		m_verticalSplitter->addWidget(m_widgetTopPart);
		m_verticalSplitter->addWidget(m_widgetBottomPart);
		m_verticalSplitter->setStretchFactor(0, 1);

		vlay->addWidget(m_verticalSplitter);

		connect(m_spButton, SIGNAL(clicked()), this, SLOT(on_southPanelButton_Clicked()));
	}
	else {
		m_spButton->setEnabled(false);
	}
	vlay->addWidget(m_spButton);
	hlay->setContentsMargins(0, 0, 0, 0);
	hlay->setMargin(0);
	hlay->setSpacing(1);
	vlay->setContentsMargins(0, 0, 0, 0);
	vlay->setMargin(0);
	vlay->setSpacing(1);
	setLayout(vlay);

	// updated check states of panel buttons based on initial widget visibility
	if (m_eastPanel != NULL)
		setPanelVisible(m_eastPanel, m_eastPanel->isVisibleTo(this));
	if (m_westPanel != NULL)
		setPanelVisible(m_westPanel, m_westPanel->isVisibleTo(this));
	if (m_southPanel != NULL)
		setPanelVisible(m_southPanel, m_southPanel->isVisibleTo(this));

	// make center panel the largest by default
	if (m_westPanel != NULL)
		m_horizontalSplitter->setStretchFactor(1, 1);
	else
		m_horizontalSplitter->setStretchFactor(0, 1);
}


void ViewBase::setPanelButtonEnabled(QWidget * w, bool enabled) {
	Q_ASSERT(w != NULL);
	if (w == m_westPanel) {
		m_wpButton->setEnabled(enabled);
	}
	else if (w == m_eastPanel) {
		m_epButton->setEnabled(enabled);
	}
	else if (w == m_southPanel) {
		m_spEastButton->setEnabled(enabled);
		m_spWestButton->setEnabled(enabled);
		m_spButton->setEnabled(enabled);
	}
}


void ViewBase::setPanelVisible(QWidget * w, bool visible) {
	Q_ASSERT(w != NULL);
	if (w == m_westPanel) {
		m_wpButton->setChecked(visible);
		on_westPanelButton_Clicked();
	}
	else if (w == m_eastPanel) {
		m_epButton->setChecked(visible);
		on_eastPanelButton_Clicked();
	}
	else if (w == m_southPanel) {
		m_widgetBottomPart->setVisible(visible);
		m_spButton->setChecked(visible);
	}
}


void ViewBase::writeSettings( QSettings& settings ){

	settings.beginGroup( objectName() );

	// store user-defined visibility properties of panels

	if (m_westPanel	!= NULL) {
		settings.setValue(  "wPV",  m_westPanel->isVisibleTo(this) );
	}

	if (m_eastPanel	!= NULL) {
		settings.setValue(  "ePV",  m_eastPanel->isVisibleTo(this) );
	}

	if (m_southPanel != NULL) {
		settings.setValue(  "sPV", m_widgetBottomPart->isVisibleTo(this) );
	}

	settings.endGroup();
}


void ViewBase::readSettings( QSettings& settings ){

	settings.beginGroup( objectName() );

	// if we have a west-panel, restore its visibility status, enable west-panel button
	// and set button status as well
	if (m_westPanel	!= NULL) {
		m_westPanel->setVisible( settings.value( "wPV", m_westPanel->isVisibleTo(this) ).toBool() );
		m_wpButton->setEnabled(true); // because we have a west-panel
		m_wpButton->setChecked(m_westPanel->isVisibleTo(this));
	}
	else {
		// no west panel, disable button
		m_wpButton->setEnabled(false);
	}

	if (m_eastPanel	!= NULL) {
		m_eastPanel->setVisible( settings.value( "ePV", m_eastPanel->isVisibleTo(this) ).toBool() );
		m_epButton->setEnabled(true); // because we have an east-panel
		m_epButton->setChecked(m_eastPanel->isVisibleTo(this));
	}

	if (m_southPanel	!= NULL) {
		m_widgetBottomPart->setVisible( settings.value( "sPV", m_southPanel->isVisibleTo(this) ).toBool() );
//		m_southPanel->setVisible( settings.value( "sPV", m_southPanel->isVisibleTo(this) ).toBool() );
		m_spEastButton->setEnabled(true);
		m_spWestButton->setEnabled(true);
		m_spButton->setEnabled(true);
		m_spButton->setChecked(m_southPanel->isVisibleTo(this));
	}

	settings.endGroup();

}

void ViewBase::saveAllOpenSubContents(){

}

/*
 *
 * private slots
 *
 */

void ViewBase::on_westPanelButton_Clicked() {
	Q_ASSERT(m_westPanel != NULL);
	int wPanelSize;
	int ePanelSize;
	// remember size of east panel, if visible and available
	if (m_epButton->isChecked())
		ePanelSize = m_eastPanel->width();
	else {
		ePanelSize = 0;
	}
	// remember size of west panel, if visible and available
	if (m_wpButton->isChecked()) {
		wPanelSize = m_lastWestPanelSize;
		m_westPanel->setVisible(true);
	}
	else {
		wPanelSize = 0;
		m_lastWestPanelSize = m_westPanel->width();
		m_westPanel->setVisible(false);
	}

	QList<int> sizes;
	sizes.append(wPanelSize);
	int w = m_horizontalSplitter->width();
	int cPanelSize = w-m_horizontalSplitter->handleWidth()-wPanelSize-ePanelSize-1; // the -1 is needed, otherwise the east-panel jumps
	cPanelSize = std::max(10, cPanelSize);
	sizes.append(cPanelSize);
	sizes.append(ePanelSize);
	m_horizontalSplitter->setSizes(sizes);
}


void ViewBase::on_eastPanelButton_Clicked() {
	Q_ASSERT(m_eastPanel != NULL);

	int wPanelSize;
	int ePanelSize;
	// remember size of westpanel, if visible and available
	if (m_wpButton->isChecked())
		wPanelSize = m_westPanel->width();
	else {
		wPanelSize = 0;
	}
	// remember size of east panel, if visible and available
	if (m_epButton->isChecked()) {
		ePanelSize = m_lastEastPanelSize;
		m_eastPanel->setVisible(true);
	}
	else {
		ePanelSize = 0;
		m_lastEastPanelSize = m_eastPanel->width();
		m_eastPanel->setVisible(false);
	}

	QList<int> sizes;
	sizes.append(wPanelSize);
	int w = m_horizontalSplitter->width();
	int cPanelSize = w-m_horizontalSplitter->handleWidth()-wPanelSize-ePanelSize-1; // the -1 is needed, otherwise the east-panel jumps
	sizes.append(cPanelSize);
	sizes.append(ePanelSize);
	m_horizontalSplitter->setSizes(sizes);
}


void ViewBase::on_southPanelButton_Clicked() {
	Q_ASSERT(m_southPanel != NULL);
	bool on = m_spButton->isChecked();
	m_widgetBottomPart->setVisible(on);
}


void ViewBase::on_spEastButton_Clicked() {
	bool on = m_spEastButton->isChecked();
	emit spEastButtonChecked(on);
}


void ViewBase::on_spWestButton_Clicked() {
	bool on = m_spWestButton->isChecked();
	emit spWestButtonChecked(on);
}

} // namespace QtExt
