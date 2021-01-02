#include "SVSimulationLocationOptions.h"
#include "ui_SVSimulationLocationOptions.h"

#include <QDir>
#include <QMessageBox>
#include <QLayout>
#include <QDesktopServices>

#include <algorithm>

#include <QtExt_Directories.h>

#include <IBK_messages.h>
#include <IBK_FormatString.h>

#include <DELPHIN_Init.h>

#include "DelProjectHandler.h"
#include "DelSettings.h"
#include "DelUndoInit.h"
#include "DelConversion.h"

const double DefaultLatitude = 52.38; // Potsdam
const double DefaultLongitude = 13.07; // Potsdam
const int DefaultTimeZoneIndex = 1; // +1 UTC Potsdam

SVSimulationLocationOptions::SVSimulationLocationOptions(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationLocationOptions)
{
	m_ui->setupUi(this);

	m_ui->widgetTreeSelectorDatabase->setManagerModeEnabled(false);
	m_ui->widgetRadioGroup->layout()->setContentsMargins(0,0,0,0);
	m_ui->verticalLayoutClimateDataDescription->setContentsMargins(0,0,0,0);
	m_ui->verticalLayoutClimateView->setContentsMargins(0,0,0,0);

	m_ui->widgetClimateView->setMinimumWidth(420);

	// populate combo boxes
	m_ui->comboBoxTimeZone->blockSignals(true);
	for (int i=-12; i<13; ++i) {
		m_ui->comboBoxTimeZone->addItem(tr("UTC %1%2").arg(i>=0 ? "+" : "").arg(i), i);
	}
	m_ui->comboBoxTimeZone->blockSignals(false);

	m_ui->lineEditLatitude->setup(-90, 90, tr("Latitude in degrees, -90 (south pole) to +90 (north pole).") );
	m_ui->lineEditLongitude->setup(-180, 180, tr("Longitude in degrees -180 (west) to +180 (east).") );

	// Albedo values from VDI 3789-2
	m_ui->comboBoxAlbedo->blockSignals(true);
	m_ui->comboBoxAlbedo->addItem(tr("Dry leveled soil - 0.2"), 0.2);
	m_ui->comboBoxAlbedo->addItem(tr("Clay soil - 0.23"), 0.23);
	m_ui->comboBoxAlbedo->addItem(tr("Light sand - 0.37"), 0.37);
	m_ui->comboBoxAlbedo->addItem(tr("Coniferous forest - 0.12"), 0.12);
	m_ui->comboBoxAlbedo->addItem(tr("Deciduous forest - 0.2"), 0.2);
	m_ui->comboBoxAlbedo->addItem(tr("Pavement - 0.15"), 0.15);
	m_ui->comboBoxAlbedo->addItem(tr("Red tiles - 0.33"), 0.33);
	m_ui->comboBoxAlbedo->addItem(tr("Wet medium grained snow - 0.64"), 0.64);
	m_ui->comboBoxAlbedo->addItem(tr("Dry new fallen snow - 0.82"), 0.82);
	m_ui->comboBoxAlbedo->setCompleter(nullptr); // no auto-completion, otherwise we have text in a value-only combo box
	m_ui->comboBoxAlbedo->blockSignals(false);

	m_ui->lineEditClimateDataFilePath->setup("", true, true, tr("Climate data container files (*.c6b *.epw *.wac);;All files (*.*)"));

	// Note: tree widget is updated (and climate directory structure is parsed) during constructor

	connect(m_ui->widgetTreeSelectorDatabase, SIGNAL(climateSelected(DelClimateFileInfo)),
			this, SLOT(onClimateSelected(DelClimateFileInfo)));
}


SVSimulationLocationOptions::~SVSimulationLocationOptions() {
	delete m_ui;
}


void SVSimulationLocationOptions::updateFromProject() {
	// this function is called from an undo-action (indirectly) and the UI must synchronize
	// to the current project() data structure. Hence, no change in UI shall result in
	// new undo actions being created -> disable all signals
	blockSignals(true);

	// empty path means none selected, yet
	IBK::Path projectClimateDataPath = project().m_init.m_climateDataContainerPath;

	// path is empty - no climate data set
	if (!projectClimateDataPath.isValid()) {
		m_ui->radioButtonNoClimateData->blockSignals(true);
		m_ui->radioButtonSelectClimate->blockSignals(true);
		m_ui->radioButtonNoClimateData->setChecked(true);
		m_ui->radioButtonSelectClimate->setChecked(false);
		m_ui->radioButtonNoClimateData->blockSignals(false);
		m_ui->radioButtonSelectClimate->blockSignals(false);
		m_ui->lineEditClimateDataFilePath->setFilename("");
		m_ui->labelFileNameReference->setText("");
		on_radioButtonSelectClimate_toggled(false); // this updates the visibility of the widgets
		blockSignals(false);
		return; // we are already done
	}

	// show climate data related widgets (we do not want to call on_radioButtonSelectClimate_toggled() here!)
	m_ui->radioButtonNoClimateData->blockSignals(true);
	m_ui->radioButtonSelectClimate->blockSignals(true);
	m_ui->radioButtonNoClimateData->setChecked(false);
	m_ui->radioButtonSelectClimate->setChecked(true);
	m_ui->radioButtonNoClimateData->blockSignals(false);
	m_ui->radioButtonSelectClimate->blockSignals(false);
	m_ui->tabWidget->setVisible(true);
	m_ui->groupBoxLocation->setVisible(true);
	m_ui->groupBoxPreview->setVisible(true);
	m_ui->groupBoxDescription->setVisible(true);


	// projectClimateDataPath can contain placeholder
	// check if it contains a path placeholder for Climate Directory or User Climate Directory
	int currentTab;
	if (projectClimateDataPath.str().find("${Climate Database}") != std::string::npos ||
		projectClimateDataPath.str().find("${User Climate Database}") != std::string::npos)
	{
		currentTab = 0;
	}
	else {
		currentTab = 1;
	}

	// special handling in case of database climate : if file path is not in the tree widget, make absolute path from filepath and treat
	// as user path

	IBK::Path absPath = DelProjectHandler::instance().replacePathPlaceholders(projectClimateDataPath);
	absPath.removeRelativeParts();
	QString absPathString = utf82QString( absPath );
	if (currentTab == 0 && !m_ui->widgetTreeSelectorDatabase->hasClimateFile( absPathString ) ) {
		projectClimateDataPath = absPath; // treat as absolute path
		currentTab = 1;
	}

	if (currentTab == 0) {
		m_ui->widgetTreeSelectorDatabase->selectClimateFile(absPathString); // no signal emitted here!
	}
	else {
		m_ui->lineEditClimateDataFilePath->setFilename(absPathString); // no signal emitted here!
		on_lineEditClimateDataFilePath_editingFinished(); // cache climate data file
		// select appropriate radio button
		if (projectClimateDataPath.str().find("${Project Directory}")==0)
			m_ui->radioButtonRelativeFilePathProjectDir->setChecked(true);
		else
			m_ui->radioButtonAbsoluteFilePath->setChecked(true);
		updateFileNameReference();
	}

	// set location data

	m_ui->checkBoxCustomLocation->blockSignals(true);
	if (project().m_init.m_flag[DELPHIN::Init::SF_USE_CUSTOM_LOCATION].isEnabled())
		m_ui->checkBoxCustomLocation->setChecked(true);
	else
		m_ui->checkBoxCustomLocation->setChecked(false);
	m_ui->checkBoxCustomLocation->blockSignals(false);

	// now update the content of the climate database view widgets
	int tabIndexBefore = m_ui->tabWidget->currentIndex();
	m_ui->tabWidget->setCurrentIndex(currentTab); // will trigger an update if currentTab != tabIndexBefore
	// if the same tab was selected again, we manually trigger the update
	if (tabIndexBefore == currentTab)
		on_tabWidget_currentChanged(currentTab);

	blockSignals(false); // done with update, allow signals from widget again
}



// *** private slots ***

void SVSimulationLocationOptions::onClimateSelected(DelClimateFileInfo climate) {
	// now update description and location group boxes
	updateDescription(climate);
	updateLocationData(climate);
	// emit change signal
	if (!climate.m_filename.isEmpty()) {
		Q_ASSERT(m_ui->tabWidget->currentIndex()==0);
		// generate relative path
		try {
			QString referencePath = climateDBFileRelative(climate.m_file.absoluteFilePath(), (climate.m_fileLocationType == DelClimateFileInfo::FLT_UserClimateDatabase));
			// signal change of climate data path, but only if different than currently selected path in
			// project(). This avoids useless sending of multiple undo-actions with same path
			sendFilePathChangedIfDifferentFromProject(referencePath);
		} catch (...) {
			// since this function should only be called with file paths relative to climate or user climate dir,
			// there should never be an exception!
			Q_ASSERT(false);
		}
	}
}


void SVSimulationLocationOptions::on_lineEditClimateDataFilePath_editingFinished() {
	// we only accept the edit when the line edit is not empty
	// if it is empty, we silently insert the default path
	if (m_ui->lineEditClimateDataFilePath->filename().isEmpty()) {
		m_ui->lineEditClimateDataFilePath->blockSignals(true);
		IBK::Path p = project().m_init.m_climateDataContainerPath;
		IBK::Path absPath = DelProjectHandler::instance().replacePathPlaceholders(p);
		m_ui->lineEditClimateDataFilePath->setFilename(utf82QString(absPath));
		m_ui->lineEditClimateDataFilePath->blockSignals(false);
	}
	// now try to read the file given in the input line
	QString newFilePath = m_ui->lineEditClimateDataFilePath->filename();
	try {
		m_userClimateInfo.readInfo(QFileInfo(newFilePath), true, true);
		m_userClimateInfo.m_name = m_userClimateInfo.m_file.baseName();
	} catch (...) {
		m_userClimateInfo = DelClimateFileInfo(); // mark as invalid
	}
	updateFileNameReference();
	updateDescription(m_userClimateInfo);
	updateLocationData(m_userClimateInfo);
}


void SVSimulationLocationOptions::on_radioButtonSelectClimate_toggled(bool checked) {
	m_ui->tabWidget->setVisible(checked);
	m_ui->groupBoxLocation->setVisible(checked);
	m_ui->groupBoxPreview->setVisible(checked);
	m_ui->groupBoxDescription->setVisible(checked);
	if (!checked) {
		// send signal to inform that climate data path was cleared
		if (project().m_init.m_climateDataContainerPath.isValid())
			sendFilePathChangedIfDifferentFromProject(QString());
	}
	else {
		// special handling when project has empty climate path and we switch to tree view
		if (!project().m_init.m_climateDataContainerPath.isValid()) {
			m_ui->widgetTreeSelectorDatabase->selectByName("DE-04-TRY-2010__Potsdam__Jahr_00000K0_00081m");
			m_ui->tabWidget->blockSignals(true);
			m_ui->tabWidget->setCurrentIndex(0);
			m_ui->tabWidget->blockSignals(false);
			on_tabWidget_currentChanged(0);
		}

		if (m_ui->tabWidget->currentIndex() == 0) {
			// get currently selected climate data path and send a path changed signal
			DelClimateFileInfo climate = m_ui->widgetTreeSelectorDatabase->selectedClimate();
			// update content of widgets according to selected climate (may be none!)
			onClimateSelected(climate);
		}
		else {
			// use already cached user climate data
			updateDescription(m_userClimateInfo);
			updateLocationData(m_userClimateInfo);
		} // user data file selected
	}
}


void SVSimulationLocationOptions::on_tabWidget_currentChanged(int index) {
	// depending on selected index
	DelClimateFileInfo climateFileInfo;
	if (index == 0) {
		climateFileInfo = m_ui->widgetTreeSelectorDatabase->selectedClimate();
	}
	else {
		climateFileInfo = m_userClimateInfo;
	}
	updateDescription(climateFileInfo);
	updateLocationData(climateFileInfo);
}


void SVSimulationLocationOptions::on_radioButtonRelativeFilePathProjectDir_toggled(bool ) {
	updateFileNameReference();
}


void SVSimulationLocationOptions::on_lineEditLatitude_editingFinished() {
	// ignore event if line edit is read only
	if (m_ui->lineEditLatitude->isReadOnly())
		return;
	// check if input is valid, if yes, emit changed signal
	if (m_ui->lineEditLatitude->isValid()) {
		double latInDeg = m_ui->lineEditLatitude->value();
		if (latInDeg < -90 || latInDeg > 90)
			return;

		// only emit if different from before
		double projectLatInDeg = project().m_init.m_para[DELPHIN::Init::SP_LATITUDE].get_value_or_default(IBK::Unit("Deg"),DefaultLatitude);
		if ( !IBK::near_equal(latInDeg, projectLatInDeg) )
			emit latitudeChanged(latInDeg);
	}
}


void SVSimulationLocationOptions::on_lineEditLongitude_editingFinished() {
	// ignore event if line edit is read only
	if (m_ui->lineEditLongitude->isReadOnly())
		return;
	// check if input is valid, if yes, emit changed signal
	if (m_ui->lineEditLongitude->isValid()) {
		double longInDeg = m_ui->lineEditLongitude->value();
		if (longInDeg < -180 || longInDeg > 360)
			return;
		if (longInDeg < 0)
			longInDeg += 360;

		double projectLongInDeg = project().m_init.m_para[DELPHIN::Init::SP_LATITUDE].get_value_or_default(IBK::Unit("Deg"),DefaultLongitude);
		if ( !IBK::near_equal(longInDeg, projectLongInDeg))
			emit longitudeChanged(longInDeg);
	}
}


void SVSimulationLocationOptions::on_comboBoxTimeZone_currentIndexChanged(int index) {
	int newTimeZone = m_ui->comboBoxTimeZone->itemData(index).toInt();
	int projectTimeZone;
	if (!project().m_init.m_intpara[DELPHIN::Init::SIP_TIMEZONE].empty())
		projectTimeZone = project().m_init.m_intpara[DELPHIN::Init::SIP_TIMEZONE].value;
	else
		projectTimeZone = DefaultTimeZoneIndex + 12;
	if (newTimeZone != projectTimeZone)
		emit timeZoneChanged(newTimeZone);
}


void SVSimulationLocationOptions::on_comboBoxAlbedo_editingFinishedSuccessfully() {
	double val = m_ui->comboBoxAlbedo->value();
	// only emit signal, if really something has changed
	double projectAlbedo = project().m_init.m_para[DELPHIN::Init::SP_ALBEDO].get_value_or_default(IBK::Unit("---"),0.2);
	if ( !IBK::near_equal(val, projectAlbedo))
		emit albedoChanged(val);
}


void SVSimulationLocationOptions::on_checkBoxCustomLocation_clicked(bool checked) {
	if (checked) {
		Q_ASSERT(!project().m_init.m_flag[DELPHIN::Init::SF_USE_CUSTOM_LOCATION].isEnabled());
		// triggered when user has checked the custom location

		// if there are valid numbers entered in the edits, keep them and send them as
		// changed values
		// otherwise use defaults
		double latInDeg = DefaultLatitude;
		if (m_ui->lineEditLatitude->isValid()) {
			double latInDegFromLineEdit = m_ui->lineEditLatitude->value();
			if (latInDegFromLineEdit >= -90 && latInDegFromLineEdit <= 90)
				latInDeg = latInDegFromLineEdit;
		}
		double longInDeg = DefaultLongitude;
		if (m_ui->lineEditLongitude->isValid()) {
			double longInDegFromLineEdit = m_ui->lineEditLongitude->value();
			if (longInDegFromLineEdit >= -180 && longInDeg <= 360)
				longInDeg = longInDegFromLineEdit;
		}
		// time zone index cannot be wrong, so just take it from the combo box
		int newTimeZone = m_ui->comboBoxTimeZone->currentData().toInt();

		// now send a change signal
		emit customLocationCheckboxChanged(true, latInDeg, longInDeg, newTimeZone);
	}
	else {
		// triggered, when user has unchecked the combo box
		emit customLocationCheckboxChanged(false,0,0,0); // here, the last three arguments are just dummies, so that we do not need two signals
	}
}

void SVSimulationLocationOptions::on_pushButtonClimateInformation_clicked() {
	QUrl climateInfo("http://www.bauklimatik-dresden.de/climatedata.php");
	bool res = QDesktopServices::openUrl(climateInfo);
	if(!res) {
		QMessageBox::warning(this, tr("Website error!"), tr("Cannot open website '%1'").arg(climateInfo.toString()));
	}
}


// *** private functions ***

QString SVSimulationLocationOptions::climateDBFileRelative(const QString& absPath, bool userClimate) const {
	IBK::Path fname(QString2Path(absPath));
	QString relativeTo = userClimate ?  DelSettings::instance().m_defaultPathPlaceholders[DelSettings::DB_UserClimate] :
										DelSettings::instance().m_defaultPathPlaceholders[DelSettings::DB_Climate];
	IBK::Path relToPath( relativeTo.toStdString() );
	IBK::Path relFname = fname.relativePath( relToPath ); // may throw
	if (userClimate)
		return "${User Climate Database}/" + QString::fromStdString(relFname.str());
	else
		return "${Climate Database}/" + QString::fromStdString(relFname.str());
}


void SVSimulationLocationOptions::updateDescription(const DelClimateFileInfo& climate) {
	// update info text on climate location
	m_ui->textBrowserDescription->clear();
	QString infoText;
	infoText = "<html><body>";
	if(!climate.m_file.isFile()) {
		infoText += "<p>" + tr("No valid climate selected.") + "</p>";
	}
	else {
		if (m_ui->tabWidget->currentIndex()==1) {
			infoText += "<p>" + tr("User climate data file.");
		}
		else {
			if (climate.m_fileLocationType == DelClimateFileInfo::FLT_UserClimateDatabase)
				infoText += "<p>" + tr("Climate data from user database.");
			else
				infoText += "<p>" + tr("Climate data from standard database.");
		}
		infoText += "<br>" + climate.m_timeBehaviour + "</p>";
		infoText += "<p>" + tr("City/Country") + ": <b>" + climate.m_city + "</b>/";
		infoText += "<b>" + climate.m_country + "</b>, ";
		infoText += tr("Source") + ": <b>" + climate.m_source + "</b><br>";
		infoText += tr("Longitude") + ": <b>" + QString("%L1 Deg").arg(climate.m_longitudeInDegree, 0, 'f', 2) + " </b>, ";
		infoText += tr("Latitude") + ": <b>" + QString("%L1 Deg").arg(climate.m_latitudeInDegree, 0, 'f', 2) + " </b>, ";
		infoText += tr("Elevation") + ": <b>" + QString("%L1 m").arg(climate.m_elevation, 0, 'f', 0) + " </b></p>";
		infoText += "<p>" + climate.m_comment + "</p>";
	}
	infoText += "</body></html>";

	m_ui->textBrowserDescription->setHtml(infoText);

	// update preview
	m_ui->widgetClimateView->setClimate(climate);
}


void SVSimulationLocationOptions::updateLocationData(const DelClimateFileInfo& climate) {
//	const char * const FUNC_ID = "[SVSimulationLocationOptions::updateLocationData]";

	int timeZoneIndex;
	// location will be set by user
	if (m_ui->checkBoxCustomLocation->isChecked()) {
		// enable editing in line edits
		m_ui->lineEditLatitude->setReadOnly(false);
		m_ui->lineEditLongitude->setReadOnly(false);
		m_ui->comboBoxTimeZone->setEnabled(true);

		double userLatitude = project().m_init.m_para[DELPHIN::Init::SP_LATITUDE].get_value_or_default(IBK::Unit("Deg"), DefaultLatitude);
		m_ui->lineEditLatitude->setText(QString("%L1").arg(userLatitude));

		double userLongitude = project().m_init.m_para[DELPHIN::Init::SP_LONGITUDE].get_value_or_default(IBK::Unit("Deg"), DefaultLongitude);
		m_ui->lineEditLongitude->setText(QString("%L1").arg(userLongitude));

		int userTimeZone;
		if(!project().m_init.m_intpara[DELPHIN::Init::SIP_TIMEZONE].empty())
			userTimeZone = project().m_init.m_intpara[DELPHIN::Init::SIP_TIMEZONE].value;
		else
			userTimeZone = DefaultTimeZoneIndex;
		timeZoneIndex = userTimeZone + 12;
		if (timeZoneIndex < 0 || timeZoneIndex > 24) {
			timeZoneIndex = 12;
		}
	}
	else {
		// data will be retrieved from climate data file
		// if the climate data file is not valid all data will be set to 0
		m_ui->lineEditLatitude->setReadOnly(true);
		m_ui->lineEditLongitude->setReadOnly(true);
		m_ui->comboBoxTimeZone->setEnabled(false);

		// invalid/missing climate?
		if (climate.isEmpty()) {
			m_ui->lineEditLatitude->setText("---");
			m_ui->lineEditLongitude->setText("---");
			timeZoneIndex = 12;
		}
		else {
			m_ui->lineEditLatitude->setText(QString("%L1").arg(climate.m_latitudeInDegree));
			m_ui->lineEditLongitude->setText(QString("%L1").arg(climate.m_longitudeInDegree));
			timeZoneIndex = climate.m_timeZone + 12;
			if (timeZoneIndex < 0 || timeZoneIndex > 24) {
				timeZoneIndex = 12;
			}
		}
	}
	m_ui->comboBoxTimeZone->blockSignals(true);
	m_ui->comboBoxTimeZone->setCurrentIndex(timeZoneIndex);
	m_ui->comboBoxTimeZone->blockSignals(false);

	// The albedo does not come from the climate file but is set by the project instead.
	m_ui->comboBoxAlbedo->blockSignals(true);
	double albedo = project().m_init.m_para[DELPHIN::Init::SP_ALBEDO].get_value_or_default(IBK::Unit("---"), 0.2);
	m_ui->comboBoxAlbedo->lineEdit()->setText( QString("%L1").arg(albedo));
	m_ui->comboBoxAlbedo->blockSignals(false);
}


void SVSimulationLocationOptions::updateFileNameReference() {
	// get absolute file path from line edit
	QString climateDataFilename = m_ui->lineEditClimateDataFilePath->filename();
	Q_ASSERT(!climateDataFilename.isEmpty());

	IBK::Path projectPath(project().filename());
	IBK::Path p = QString2Path(climateDataFilename);
	std::string errstr;
	// use absolute path if either radio button is selected or project hasn't been saved yet
	if (m_ui->radioButtonRelativeFilePathProjectDir->isChecked() &&
		projectPath.isValid() &&
		p.canCreateRelativePath(projectPath.parentPath(), errstr))
	{
		p = "${Project Directory}" / p.relativePath(projectPath.parentPath());
		climateDataFilename = utf82QString(p);
	}
	m_ui->labelFileNameReference->setText(climateDataFilename);
	sendFilePathChangedIfDifferentFromProject(climateDataFilename); // trigger emit when filepath has changed
}


void SVSimulationLocationOptions::sendFilePathChangedIfDifferentFromProject(const QString & newFilePath) {
	if (utf82QString(project().m_init.m_climateDataContainerPath) != newFilePath)
		emit climateDataPathChanged(newFilePath);
}

