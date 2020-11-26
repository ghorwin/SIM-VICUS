#include "SVPreferencesPageStyle.h"
#include "ui_SVPreferencesPageStyle.h"

#include "SVSettings.h"
#include "SVStyle.h"

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


void SVPreferencesPageStyle::on_comboBoxTheme_activated(const QString &theme) {
	// no checks necessary
	SVSettings & setting = SVSettings::instance();
	SVStyle & style = SVStyle::instance();
	if ( theme == "White" ) {
		setting.m_theme = SVSettings::TT_White;
		style.setStyle(false);
	}
	else if (theme == "Dark" ) {
		setting.m_theme = SVSettings::TT_Dark;
		style.setStyle(true);
	}
	// now apply the style
	emit styleChanged();
}
