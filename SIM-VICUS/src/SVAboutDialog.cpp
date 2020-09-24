#include "SVAboutDialog.h"
#include "ui_SVAboutDialog.h"

#include <VICUS_Constants.h>

#include <QtExt_LanguageHandler.h>

SVAboutDialog::SVAboutDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVAboutDialog)
{
	m_ui->setupUi(this);

	setWindowTitle(QString("SIM-VICUS %1").arg(VICUS::LONG_VERSION));

	m_ui->label->setPixmap( QPixmap(":/gfx/splashscreen/splashScreen.png"));

	layout()->setMargin(0);
	layout()->setSizeConstraint( QLayout::SetFixedSize );
}


SVAboutDialog::~SVAboutDialog() {
	delete m_ui;
}
