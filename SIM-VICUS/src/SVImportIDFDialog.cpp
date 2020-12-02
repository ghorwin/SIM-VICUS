#include "SVImportIDFDialog.h"
#include "ui_SVImportIDFDialog.h"

#include <QMessageBox>

#include "SVProjectHandler.h"

#include <EP_Project.h>

SVImportIDFDialog::SVImportIDFDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVImportIDFDialog)
{
	m_ui->setupUi(this);
}


SVImportIDFDialog::~SVImportIDFDialog() {
	delete m_ui;
}


SVImportIDFDialog::ImportResults SVImportIDFDialog::import(const QString & fname) {

	// read IDF file

	try {
		EP::Project prj;
		prj.readIDF(IBK::Path(fname.toStdString()));

		// now transfer data to temporary VICUS project structure.
		m_importedProject = VICUS::Project(); // clear data from previous import
		transferData(prj);
	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical(this, tr("Import error"), tr("Error parsing IDF file:\n%1").arg(ex.what()));
		return ImportCancelled;
	}

	// if successful, show dialog

	// merge project is only active if we have a project

	m_ui->pushButtonMerge->setEnabled( SVProjectHandler::instance().isValid() );

	int res = exec();
	if (res == QDialog::Rejected)
		return ImportCancelled;

	return m_returnCode;
}


void SVImportIDFDialog::transferData(const EP::Project & prj) {
	FUNCID(SVImportIDFDialog::transferData);

	VICUS::Project & vp = m_importedProject; // readability improvement
	vp.m_buildings.resize(1);
	vp.m_buildings[0].m_buildingLevels.resize(1);
	VICUS::BuildingLevel & bl = vp.m_buildings[0].m_buildingLevels[0];

	std::map<std::string, unsigned int> mapZoneID;
	// import all zones
	for (const EP::Zone & z : prj.m_zones) {
		VICUS::Room r;
		r.m_id = r.uniqueID();
		r.m_displayName = QString::fromStdString(z.m_name);

		// remember zone name - id association
		if (mapZoneID.find(z.m_name) != mapZoneID.end())
			throw IBK::Exception(IBK::FormatString("Duplicate zone ID name '%1'").arg(z.m_name), FUNC_ID);
		mapZoneID[z.m_name] = r.m_id;

		// transfer attributes

		// TODO : Dirk

		// add zone
		bl.m_rooms.push_back(r);
	}

}

void SVImportIDFDialog::on_pushButtonReplace_clicked() {
	m_returnCode = ReplaceProject;
	accept();
}

void SVImportIDFDialog::on_pushButtonMerge_clicked() {
	m_returnCode = MergeProjects;
	accept();
}
