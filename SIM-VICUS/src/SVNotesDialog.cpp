#include "SVNotesDialog.h"
#include "ui_SVNotesDialog.h"

#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVNotesDialog::SVNotesDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNotesDialog)
{
	m_ui->setupUi(this);
	// Connect to project handler
	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified, this, &SVNotesDialog::onModified);
}

SVNotesDialog::~SVNotesDialog() {
	delete m_ui;
}

void SVNotesDialog::updateNotesFromPrj() {
	VICUS::Project &prj = const_cast<VICUS::Project &>(SVProjectHandler::instance().project() );
	// Save updated comment
	m_ui->plainTextEditNotes->setPlainText(QString::fromStdString(prj.m_projectInfo.m_comment) );
}

void SVNotesDialog::onModified(int modificationType, ModificationInfo *) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::AllModified:
			// Update the project notes.
			updateNotesFromPrj();
		break;

		default: ; // just to make compiler happy
	} // switch
}


void SVNotesDialog::on_pushButtonSave_clicked() {
	VICUS::Project &prj = const_cast<VICUS::Project &>(SVProjectHandler::instance().project() );
	// Save updated comment
	prj.m_projectInfo.m_comment = m_ui->plainTextEditNotes->toPlainText().toStdString();
	accept();
}


void SVNotesDialog::on_pushButtonCancel_clicked() {
	reject();
}


