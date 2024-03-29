#ifndef SVPROPRESULTSVIEWWIDGETH
#define SVPROPRESULTSVIEWWIDGETH

#include <QWidget>
#include <QDir>
#include <QDateTime>

#include <NANDRAD_LinearSplineParameter.h>

#include <VICUS_Constants.h>

#include "SVColorMap.h"


namespace Ui {
class SVPropResultsWidget;
}

class ModificationInfo;

/*! Widget showing options to highlight results in false color mode in UI.

	The widget maintains a state of output files in currently selected output directory.
	For each output (tsv) file, a data store is kept with all time series in the file.

	Initially, only the file headers are parsed and the quantities are extracted (for example,
	AirTemperature, where there may be several time series for several rooms). For each quantity
	the respective source file (can be only one) is stored.

	When users select a quantity, it will be selected for coloring if cached already. Otherwise nothing happens.

	When users double-clicks a quantity, the respective file is read (in readDataFile()) and the
	data is cached in memory. Then, the double-clicked quantity is made active and shown.

	Then, the values are converted to colors depending on the selected color map and the geometrical elements
	are colored (in updateColors()). The scene is then told to update its color buffers via setting the
	view state (again).
*/
class SVPropResultsWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropResultsWidget(QWidget *parent = nullptr);
	~SVPropResultsWidget() override;

	/*! This function is called whenever the property widget is shown.
		It initializes the default results directory on first call,
		and afterwards re-reads the output directory as if user would
		have pressed "Refresh" button manually.
	*/
	void refreshDirectory();

	/*! Sets colors to all VICUS objects, based on previously found ids */
	void updateColors(const double & currentTime);

private slots:

	void onModified(int modificationType, ModificationInfo *data);

	void onTimeSliderCutValueChanged(double currentTime);

	void on_pushButtonSetGlobalMinMax_clicked();
	void on_pushButtonSetLocalMinMax_clicked();

	/*! Activates the currently selected quantity if in cache. If not in cache, does nothing. */
	void on_tableWidgetAvailableResults_itemSelectionChanged();

	/*! Attempts to load the data file containing the selected quantity. If successful, shows
		the quantity (by selecting the row).
	*/
	void on_tableWidgetAvailableResults_cellDoubleClicked(int row, int column);

	void on_toolButtonSetDefaultDirectory_clicked();
	void on_pushButtonRefreshDirectory_clicked();

	void on_lineEditMaxValue_editingFinishedSuccessfully();
	void on_lineEditMinValue_editingFinishedSuccessfully();

	void on_pushButtonSetDefaultColormap_clicked();

	void on_pushButtonReadColormap_clicked();

	void on_pushButtonSaveColormap_clicked();

	void on_checkBoxConvertToAbsolute_stateChanged(int arg1);

	void on_pushButtonSetColormapViridis_clicked();

	void on_pushButtonSetColormapSpectral_clicked();

	void on_pushButtonJumpToMax_clicked();

	void on_pushButtonJumpToMin_clicked();

	void on_pushButtonFindMaxObject_clicked();

	void on_pushButtonFindMinObject_clicked();

	void on_resultsDir_editingFinished();

private:

	void clearUi();

	/*! Caches the content of a single output file. */
	struct ResultDataSet {
		enum FileStatus {
			/*! File hasn't been read, yet. */
			FS_Unread,
			/*! File on disk is older/same age as m_timeStampLastUpdated */
			FS_Current,
			/*! File on disk is newer than read data set, update needed. */
			FS_Outdated,
			/*! When file cannot be read (again) or file is missing. */
			FS_Missing
		};

		/*! File name, relative to 'results/' directory. */
		QString		m_filename;
		/*! Stores date/time when data from this file was read last.
			Time stamp is invalid, if data hasn't been read, yet.
		*/
		QDateTime	m_timeStampLastUpdated;

		/*! The file status. */
		FileStatus	m_status = FS_Unread;
	};


	enum ResultFileType {
		FT_TSV,
		FT_BTF,
		FT_None
	};

	/*! Parses the substitution file ('objectref_substitutions.txt') and the header of all tsv files in the currently selected results directory.
		Updates the table widget if results have been found */
	void readResultsDir();

	/*! Updates fonts, text colors and icons in table based on current cache state.
		\note Does not change selection and does not trigger any side effects.
	*/
	void updateTableWidgetFormatting();

	/*! Parses the entire tsv file. Stores all outputs that have been found in this tsv file.
		If successful indicates newly cached data through green flag in the table widget.
	*/
	void readDataFile(const QString & filename);

	/*! Determine min/max values of current output. If localMinMax==true, the min/max of current time point are determined, otherwise the min/max of entire spline are determined */
	void setCurrentMinMaxValues(bool localMinMax=false);

	/*! Reads colormap from xml file */
	bool readColorMap(const QString &filename);

	/*! Interpolates color for y considering m_currentMin, m_currentMax */
	void interpolateColor(double y, QColor &col) const;

	/*! creates undo action to select object with targetId, also sets the camera to find object */
	void selectTargetObject(unsigned int targetId);

	/*! Update lineedit with value of currently selected object */
	void updateLineEditCurrentValue();

	void onSelectionChanged();

	Ui::SVPropResultsWidget							*m_ui;

	/*! Currently selected results directory, updated in readResultsDir(), does not contain trailing 'results/' subdirectory. */
	QDir											m_resultsDir;

	/*! List of all output files that are in resultsDir, updated in readResultsDir(). */
	QList<ResultDataSet>							m_outputFiles;

	/*! Maps an output variable (extracted from caption in tsv-file) to the file index in vector m_outputFiles. */
	std::map<QString, unsigned int>					m_outputVariable2FileIndexMap;

	/*! Stores VICUS Object Ids for each output property (effectively the content of the objectref_substitutions.txt file).
		Key is the name that appears in the beginning of a caption of a tsv-file, for example: "BuildingName.E0.WE0.0_Bath(ID=100)" where
		the caption is "BuildingName.E0.WE0.0_Bath(ID=100).AirTemperature"
	*/
	std::map<QString, unsigned int>					m_objectName2Id;

	/*! Holds map for each output property with key being VICUS Object Id and value being the according results values as linear spline. */
	std::map<QString, std::map<unsigned int, NANDRAD::LinearSplineParameter> >	m_allResults;

	/*! The currently selected output property/quantity (extracted from caption in TSV files). */
	QString											m_currentOutputQuantity;
	QString											m_currentOutputUnit;

	/*! The currently selected filter. */
	QString											m_currentFilter;

	/*! Min/Max values and colors. */
	double											m_currentMin = 0;
	double											m_currentMax = 1;
	unsigned int									m_currentMinIdx = 0;
	unsigned int									m_currentMaxIdx = 0;

	/*! Currently used color map */
	SVColorMap										m_colorMap;

	QString											m_lastOpenFileLocation;

	/*! VICUS Object Id of currently selected object */
	unsigned int									m_selectedObjectId = VICUS::INVALID_ID;

	ResultFileType									m_resultFileType = FT_None;

};

#endif // SVRESULTSVIEWWIDGETH
