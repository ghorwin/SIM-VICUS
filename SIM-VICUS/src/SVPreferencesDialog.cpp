#include "SVPreferencesDialog.h"
#include "ui_SVPreferencesDialog.h"

#include "SVPreferencesPageTools.h"
#include "SVPreferencesPageStyle.h"
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QShortcut>
#include <QDebug>

SVPreferencesDialog::SVPreferencesDialog(QWidget * parent) :
	QWidget(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
	m_ui(new Ui::SVPreferencesDialog)
{
	m_ui->setupUi(this);


	// setup configuration pages

	m_pageTools = new SVPreferencesPageTools(this);
	m_pageStyle = new SVPreferencesPageStyle(this);
	m_ui->tabWidget->addTab(m_pageTools, tr("External tools"));
	m_ui->tabWidget->addTab(m_pageStyle, tr("Style"));

	// ... other pages
}


SVPreferencesDialog::~SVPreferencesDialog() {
	delete m_ui;
}


bool SVPreferencesDialog::edit(int initialPage) {
	// transfer settings data to User Interface
	updateUi();

	m_ui->tabWidget->setCurrentIndex(initialPage);

	show(); // and show the dialog
}


// ** private functions **

void SVPreferencesDialog::updateUi() {
	m_pageTools->updateUi();
	m_pageStyle->updateUi();
	// ... other pages
}


