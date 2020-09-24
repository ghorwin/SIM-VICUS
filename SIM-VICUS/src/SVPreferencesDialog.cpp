#include "SVPreferencesDialog.h"
#include "ui_SVPreferencesDialog.h"

#include "SVPreferencesPageTools.h"
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QShortcut>

SVPreferencesDialog::SVPreferencesDialog(QWidget * parent) :
	QDialog(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
	m_ui(new Ui::SVPreferencesDialog)
{
	m_ui->setupUi(this);

	// setup configuration pages

	m_pageTools = new SVPreferencesPageTools(this);
	m_ui->tabWidget->addTab(m_pageTools, tr("External tools"));

	// ... other pages

	setMinimumSize(750,670);
}


SVPreferencesDialog::~SVPreferencesDialog() {
	delete m_ui;
}


bool SVPreferencesDialog::edit(int initialPage) {
	// transfer settings data to User Interface
	updateUi();

	m_ui->tabWidget->setCurrentIndex(initialPage);

	// execute dialog and pass on result
	return (exec() == QDialog::Accepted);
}



// ** protected functions **

void SVPreferencesDialog::accept() {
	if (!storeConfig())
		return;

	QDialog::accept();
}


// ** private functions **

void SVPreferencesDialog::updateUi() {
	m_pageTools->updateUi();
	// ... other pages
}


bool SVPreferencesDialog::storeConfig() {
	if (!m_pageTools->storeConfig()) {
		m_ui->tabWidget->setCurrentWidget(m_pageTools);
		return false;
	}
	// ... other pages
	return true;
}

