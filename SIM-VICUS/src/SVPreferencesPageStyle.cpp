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

	if ( theme == "White" ) {
		style.setStyle(SVSettings::TT_White);
	}
	else if (theme == "Dark" ) {
		style.setStyle(SVSettings::TT_Dark);
	}
	// now apply the style
	emit styleChanged();
}
