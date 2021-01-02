#ifndef SVSimulationLocationOptionsH
#define SVSimulationLocationOptionsH

#include <QWidget>

#include <IBK_Path.h>

namespace Ui {
	class SVSimulationLocationOptions;
}

#include "SVClimateFileInfo.h"

/*! Widget that allows users to select climate data.

	The widget is configured from project settings according to the following rules:

	From project data:
	- climate data path empty	-> radio button "No climate" checked, nothing visible
								-> otherwise radio button "Select climate" checked, rest visible

	- climate path has placeholder climate dir or user climate dir -> select tab "Climate database,
		otherwise select tab user data
	- if in "Climate database" tab, try to select climate in tree view -> if this fails (data file missing, wrong path),
		switch to "User data file", convert path to absolute path and insert in line (this shows users that the selected file
		in project is invalid and doesn't alter data in view)
	- if in "user data view", convert to absolute file path and insert in line, detect presence of "Project Directory" placeholder
		and check associated radio button

	- if project file has properties LONGITUDE or LATITUDE set, check the checkbox "Custom location" and set the data from these values,
		otherwise disable these inputs and update the content from climate data selection

	During update of UI, none of the changes made must signal a change of data and create an undo action! Since change signals are emitted
	from this widget only, a widget-level blockSignals() before and after the update is sufficient.

	The following changes are monitored and cause undo-actions to be created:
	- top radio buttons -> clear/set climate data path (no need to store old path, since we have undo)
	- tab widget selection -> when switching to climate data view and selecting a climate file, and
		when switching to user view and the filename edit is not empty
	- when user has selected/entered a new filename (editingFinished())
	- when user has change the "reference type" radio button
	- when user has selected a new climate data file in tree view (whether it is valid or not)
	- when user has checked/unchecked the custom location checkbox
	- when user has entered/changed location parameters
*/
class SVSimulationLocationOptions : public QWidget {
	Q_OBJECT

public:
	explicit SVSimulationLocationOptions(QWidget *parent = nullptr);
	~SVSimulationLocationOptions();

	/*! Updates content of the widget to the current content of the project() data structure.
		During this operation no change signals are send.
	*/
	void updateFromProject();

	/*! Set visibilty of preview widget.*/
//	void setPreviewVisible(bool visible);

signals:
	/*! Emitted when the latitude has changed, lat in [Deg].
		Connected to DelProjectPropertiesView.
	*/
	void latitudeChanged(double latitudeInDegrees);

	/*! Emitted when the longitude has changed, longitude in [Deg].
		Connected to DelProjectPropertiesView.
	*/
	void longitudeChanged(double longitudeInDegrees);

	/*! Emitted when time zone has changed, time zone in [-12..12].
		Connected to DelProjectPropertiesView.
	*/
	void timeZoneChanged(int timeZone);

	/*! Emitted when user selects a new climate data file path.
		When path is empty, user has selected the "No climate" radio button.
		If in "User data file" tab, and user clears filename, no signal is send.
		Connected to DelProjectPropertiesView::onClimateDataPathChanged().
		\param climateDataPath The path to the selected climate data file, may contain placeholders.
	*/
	void climateDataPathChanged(QString climateDataPath);

	/*! Emitted when the albedo has changed, albedo in [0..1].
		Connected to DelProjectPropertiesView.
	*/
	void albedoChanged(double albedo);

	/*! Emitted, when user has toggled the custom location radio button.
		If custom location is enabled, also the currently set parameters (e.g. from climate data file) are transferred.
	*/
	void customLocationCheckboxChanged(bool customLocationChecked, double latitude, double longitude, int timeZone);

private slots:
	/*! Triggered, when user has selected a new climate in climate data base tree widget.
		This function is also called, when user has switch to climate tree mode or
		has selected a new file. It updates the description text and location data.
		Also emits signal climateDataPathChanged() when new climate path is different
		from the currently stored path in the project.
	*/
	void onClimateSelected(SVClimateFileInfo climate);

	/*! Update the description and path reference based on new climate file name.
		If user has cleared the file path, this function simply restores the
		original file name from project in the line edit. Otherwise,
		the climate file is read and cached in m_userClimateInfo variable for access
		in updateDescription() and updateLocation().
	*/
	void on_lineEditClimateDataFilePath_editingFinished();

	/*! Triggered, when user toggles between climate or no climate.
		Updates view state based on current project data.
		Emits signal climateDataPathChanged().
	*/
	void on_radioButtonSelectClimate_toggled(bool checked);

	/*! Triggered, when user switches between climate data or user file.
		Updates content of view based on current project data.
		Emits signal climateDataPathChanged().
	*/
	void on_tabWidget_currentChanged(int index);

	/*! Triggered when user toggles relative/absolute path setting. */
	void on_radioButtonRelativeFilePathProjectDir_toggled(bool checked);

	/*! Checks validity of value and emits latitudeChanged if values is changed.*/
	void on_lineEditLatitude_editingFinished();

	/*! Checks validity of value and emits longitudeChanged if values is changed.*/
	void on_lineEditLongitude_editingFinished();

	/*! Checks validity of value and emits timeZoneChanged if values is changed.*/
	void on_comboBoxTimeZone_currentIndexChanged(int index);

	/*! Emit albedoChanged.*/
	void on_comboBoxAlbedo_editingFinishedSuccessfully();

	/*! Emits customLocationCheckboxChanged() if checked/unchecked. */
	void on_checkBoxCustomLocation_clicked(bool checked);

	void on_pushButtonClimateInformation_clicked();

private:

	/*! Update the text in the description field.*/
	void updateDescription(const SVClimateFileInfo& climate);

	/*! Updates the location settings based on the given climate.*/
	void updateLocationData(const SVClimateFileInfo& climate);

	/*! Create a climate path relative to database or user database.
		Throws an IBK::Exception, if a relative path cannot be created (and should be handled accordingly!).
	*/
	QString climateDBFileRelative(const QString& absPath, bool userClimate) const;

	/*! Update name in reference label based on relative path settings and currently entered path in line edit for filename. */
	void updateFileNameReference();

	/*! Only function to emit the signal climateDataPathChanged().
		Only emits the signal, if the new path (which may contain path placeholders) is different from the one
		currently set in the project.
	*/
	void sendFilePathChangedIfDifferentFromProject(const QString & newFilePath);


	Ui::SVSimulationLocationOptions *m_ui;

	/*! Cached/updated in on_lineEditClimateDataFilePath_editingFinished(), whenever user has changed
		the file path. Only updated if a path has been entered.
	*/
	SVClimateFileInfo				m_userClimateInfo;
};

#endif // SVSimulationLocationOptionsH
