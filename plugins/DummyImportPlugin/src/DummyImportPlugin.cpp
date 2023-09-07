#include "DummyImportPlugin.h"

#include <QMessageBox>

int DummyImportPlugin::showSettingsDialog(QWidget * parent) {
	// spawn settings dialog
	QMessageBox::information(parent, QString(), tr("Here is the settings dialog for the plugin."));
	return NoUpdate;
}


QString DummyImportPlugin::importMenuCaption() const {
	return tr("Dummy project import");
}


bool DummyImportPlugin::import(QWidget * parent, QString & p) {
	// spawn import dialog and populate project
	QMessageBox::information(parent, QString(), tr("Here is the import dialog for the plugin."));

	p = "I should contain the text of a VICUS project file";

	return true;
}
