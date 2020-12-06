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

	m_ui->comboBoxTheme->addItem("White");
	m_ui->comboBoxTheme->addItem("Dark");

	SVSettings & s = SVSettings::instance();
	m_ui->comboBoxTheme->setCurrentIndex(s.m_theme);
}


SVPreferencesPageStyle::~SVPreferencesPageStyle() {
	delete m_ui;
}


void SVPreferencesPageStyle::updateUi() {
	SVSettings & s = SVSettings::instance();
	// transfer data to Ui
	m_ui->comboBoxTheme->setCurrentIndex(s.m_theme);
	const SVSettings::ThemeSettings & ts = s.m_themeSettings[s.m_theme];
	m_ui->pushButtonMajorGridColor->setColor(ts.m_majorGridColor);
	m_ui->pushButtonMinorGridColor->setColor(ts.m_minorGridColor);
	m_ui->pushButtonSceneBackgroundColor->setColor(ts.m_sceneBackgroundColor);
	m_ui->pushButtonSelectedSurfaceColor->setColor(ts.m_selectedSurfaceColor);
}


bool SVPreferencesPageStyle::storeConfig() {
	// no checks necessary
	SVSettings & s = SVSettings::instance();

	if ( m_ui->comboBoxTheme->currentText() == "White" ) {
		s.m_theme = SVSettings::TT_White;
	}
	if ( m_ui->comboBoxTheme->currentText() == "Dark" ) {
		s.m_theme = SVSettings::TT_Dark;
	}

	return true;
}

bool SVPreferencesPageStyle::rejectConfig() {
	// no checks necessary
	SVSettings & setting = SVSettings::instance();
	SVStyle & style = SVStyle::instance();

	style.setStyle(setting.m_theme);

	return true;
}


void SVPreferencesPageStyle::on_comboBoxTheme_activated(const QString &theme) {
	// no checks necessary
	SVStyle & style = SVStyle::instance();
	SVSettings & s = SVSettings::instance();

	/// \todo Improve this with item data
	if ( theme == "White" ) {
		s.m_theme = SVSettings::TT_White;
	}
	else if (theme == "Dark" ) {
		s.m_theme = SVSettings::TT_Dark;
	}

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
