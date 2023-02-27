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

#include "SVAboutDialog.h"
#include "ui_SVAboutDialog.h"

#include <VICUS_Constants.h>

#include <QtExt_LanguageHandler.h>

#include <QRandomGenerator>

#include "SVStyle.h"
#include "SVSettings.h"

SVAboutDialog::SVAboutDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVAboutDialog)
{
	m_ui->setupUi(this);

	setWindowTitle(QString("SIM-VICUS %1").arg(VICUS::LONG_VERSION));

	int imageCount = 6;

	QPixmap pixmap;

#if QT_VERSION >= 0x050A00
	int pixmapIdx = QRandomGenerator::global()->bounded(0,imageCount);
#else

	std::srand(std::time(nullptr));
	int pixmapIdx = std::rand()*imageCount/RAND_MAX;
#endif

	pixmap.load(QString(":/gfx/splashscreen/SIMVICUS-Logo-Startscreen-%1.png").arg(pixmapIdx));

	// is needed for high dpi screens to prevent bluring
	pixmap.setDevicePixelRatio(SVSettings::instance().m_ratio);

	// Load custom font
	m_ui->label->setPixmap(pixmap);
	QString labelStyle(
				"font-size:12pt; color: #3caed0; text-decoration:none"
				);

	SVStyle::formatWelcomePage(labelStyle);

	QLabel * linkLabel = new QLabel( QString("<a href=\"https://sim-vicus.de\"><span style=\"%1\">https://sim-vicus.de</span></a>").arg(labelStyle));
	linkLabel->setParent(this);
	linkLabel->resize(400,25);
	linkLabel->setAutoFillBackground(false);
	linkLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	linkLabel->setOpenExternalLinks(true);
	linkLabel->move(250,270);
	linkLabel->setAttribute(Qt::WA_TranslucentBackground);

	layout()->setMargin(0);
	layout()->setSizeConstraint( QLayout::SetFixedSize );
}


SVAboutDialog::~SVAboutDialog() {
	delete m_ui;
}

