#include "SVImportDxfDialog.h"
#include "ui_SVImportDxfDialog.h"

#include "SVSettings.h"
#include "SVUndoAddDrawing.h"

SVImportDxfDialog::SVImportDxfDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVImportDxfDialog)
{
	m_ui->setupUi(this);

	m_ui->lineEditFileName->setup(m_lastFilePath, true, true, tr("DXF-Files (*.dxf)"),
								   SVSettings::instance().m_dontUseNativeDialogs);
}

SVImportDxfDialog::~SVImportDxfDialog()
{
	delete m_ui;
}

void SVImportDxfDialog::run() {

	if (exec()) {

		readDxfFile();

		SVUndoAddDrawing *undo = new SVUndoAddDrawing("", m_drawing);
		undo->push();
	}
}


void SVImportDxfDialog::readDxfFile() {

	m_drawing =

}

