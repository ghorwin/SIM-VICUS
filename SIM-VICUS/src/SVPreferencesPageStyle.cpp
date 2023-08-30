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

#include "SVPreferencesPageStyle.h"
#include "ui_SVPreferencesPageStyle.h"

#include "SVSettings.h"
#include "SVStyle.h"
#include "SVMainWindow.h"

SVPreferencesPageStyle::SVPreferencesPageStyle(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPreferencesPageStyle)
{
	m_ui->setupUi(this);

	m_ui->pushButtonMajorGridColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonMinorGridColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonSceneBackgroundColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);
	m_ui->pushButtonSelectedSurfaceColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->labelDarkPreview->setPixmap(QPixmap(":/gfx/style/dark.png").scaledToHeight(100, Qt::SmoothTransformation));
	m_ui->labelWhitePreview->setPixmap(QPixmap(":/gfx/style/white.png").scaledToHeight(100, Qt::SmoothTransformation));
}


SVPreferencesPageStyle::~SVPreferencesPageStyle() {
	delete m_ui;
}


void SVPreferencesPageStyle::updateUi() {
	SVSettings & s = SVSettings::instance();
	// transfer data to Ui
	m_ui->radioButtonDark->setChecked(s.m_theme == SVSettings::TT_Dark);
	m_ui->radioButtonWhite->setChecked(s.m_theme == SVSettings::TT_White);

	const SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	m_ui->pushButtonMajorGridColor->setColor(ts.m_majorGridColor);
	m_ui->pushButtonMinorGridColor->setColor(ts.m_minorGridColor);
	m_ui->pushButtonSceneBackgroundColor->setColor(ts.m_sceneBackgroundColor);
	m_ui->pushButtonSelectedSurfaceColor->setColor(ts.m_selectedSurfaceColor);
	m_ui->checkBoxScaling->blockSignals(true);
	m_ui->checkBoxScaling->setChecked(s.m_useHighDPIScaling);
	m_ui->checkBoxScaling->blockSignals(false);
}


void SVPreferencesPageStyle::on_pushButtonSceneBackgroundColor_colorChanged() {
	SVSettings & s = SVSettings::instance();
	SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	ts.m_sceneBackgroundColor = m_ui->pushButtonSceneBackgroundColor->color();
	emit styleChanged();
	qApp->processEvents();
}


void SVPreferencesPageStyle::on_pushButtonMajorGridColor_colorChanged() {
	SVSettings & s = SVSettings::instance();
	SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	ts.m_majorGridColor = m_ui->pushButtonMajorGridColor->color();
	emit styleChanged();
	qApp->processEvents();
}


void SVPreferencesPageStyle::on_pushButtonMinorGridColor_colorChanged() {
	SVSettings & s = SVSettings::instance();
	SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	ts.m_minorGridColor = m_ui->pushButtonMinorGridColor->color();
	emit styleChanged();
	qApp->processEvents();
}


void SVPreferencesPageStyle::on_pushButtonSelectedSurfaceColor_colorChanged() {
	SVSettings & s = SVSettings::instance();
	SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	ts.m_selectedSurfaceColor = m_ui->pushButtonSelectedSurfaceColor->color();
	emit styleChanged();
	qApp->processEvents();
}

void SVPreferencesPageStyle::on_pushButtonDefault_clicked() {
	SVSettings & s = SVSettings::instance();
	SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	ts.setDefaults(s.m_theme);
	m_ui->pushButtonMajorGridColor->setColor(ts.m_majorGridColor);
	m_ui->pushButtonMinorGridColor->setColor(ts.m_minorGridColor);
	m_ui->pushButtonSceneBackgroundColor->setColor(ts.m_sceneBackgroundColor);
	m_ui->pushButtonSelectedSurfaceColor->setColor(ts.m_selectedSurfaceColor);
	emit styleChanged();
	qApp->processEvents();
}

void SVPreferencesPageStyle::on_checkBoxScaling_toggled(bool useScaling) {
	SVSettings::instance().m_useHighDPIScaling = useScaling;
	QMessageBox::information(this, tr("High Resolution Scaling"), tr("Scaling of Interface will apply after program restart.") );
}

void SVPreferencesPageStyle::on_radioButtonDark_toggled(bool useDarkTheme) {
	// no checks necessary
	SVStyle & style = SVStyle::instance();
	SVSettings & s = SVSettings::instance();

	if (useDarkTheme)
		s.m_theme = SVSettings::TT_Dark;
	else
		s.m_theme = SVSettings::TT_White;

	style.setStyle(s.m_theme);

	// transfer theme-specific settings to UI
	const SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	m_ui->pushButtonMajorGridColor->setColor(ts.m_majorGridColor);
	m_ui->pushButtonMinorGridColor->setColor(ts.m_minorGridColor);
	m_ui->pushButtonSceneBackgroundColor->setColor(ts.m_sceneBackgroundColor);
	m_ui->pushButtonSelectedSurfaceColor->setColor(ts.m_selectedSurfaceColor);

	// now apply the style
	emit styleChanged();
}

