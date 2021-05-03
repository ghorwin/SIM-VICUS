#ifndef SVShadingCalculationDialogH
#define SVShadingCalculationDialogH

#include <QDialog>
#include <QProgressDialog>
#include <QElapsedTimer>

#include <NANDRAD_SimulationParameter.h>

#include <SVSimulationStartNandrad.h>

#include <SH_StructuralShading.h>


namespace Ui {
	class SVShadingCalculationDialog;
}

/*! Starts shading calculation. */
class SVShadingCalculationDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVShadingCalculationDialog(QWidget *parent = nullptr);
	~SVShadingCalculationDialog();

	/*! Defines the specific output type */
	enum OutputType {
		TsvFile,		///< write output to tsv file
		D6oFile,		///< write output to d6o file
		D6bFile,			///< write output to d6b file
		NUM_OT
	};

	/*! Defines the specific output type */
	enum DetailType {
		Fast,			///< fast calculation parameters
		Detailed,		///< detailed calculation paramtesers
		Manual,			///< manually defined parameters
		NUM_DT
	};

	int edit();

private slots:
	void on_pushButtonCalculate_clicked();

	void on_lineEditStartDate_editingFinished();

	void on_lineEditEndDate_editingFinished();

	void on_lineEditDuration_editingFinishedSuccessfully();

	void on_lineEditGridSize_editingFinished();

	void on_lineEditSunCone_editingFinished();

	void on_comboBoxFileType_currentIndexChanged(int index);

	void on_pushButtonChangeTime_clicked();

	void on_radioButtonFast_toggled(bool checked);

	void on_radioButtonManual_toggled(bool checked);

	void on_radioButtonDetailed_toggled(bool checked);

private:
	/*! Function that calculates the shading factors for selected outside surfaces
		Returns false if some error occurred during creation of the NANDRAD project.
	*/
	void calculateShadingFactors();


	void updateTimeFrameEdits();

	/*! Sets the simulation parameters */
	void setSimulationParameters(const DetailType &dt);


	Ui::SVShadingCalculationDialog		*m_ui;								///< pointer to UI

	NANDRAD::SimulationParameter		*m_simulationParameter = nullptr;	///< stores pointer to NANDRAD simulation parameters
	SVSimulationStartNandrad			*m_simStartWidget = nullptr;		///< stores pointer to SV Simulation parameters

	double								m_gridSize = 0.1;					///< size of grid
	double								m_sunCone = 3;						///< half opening angle of the cone for sun mapping

	OutputType							m_outputType = OutputType::TsvFile;	///< stores the output file type

	std::vector<const VICUS::Surface*>	m_selSurfaces;						///< vector with selected surfaces
	std::vector<const VICUS::Surface*>	m_selObstacles;						///< vector with selected dump geometry (obstacles)

	/*! Local copy of our project data, modified in dialog and synced with global
		project when dialog is confirmed.
	*/
	VICUS::Project						m_localProject;

	SH::StructuralShading				m_shading;							///< shading object, that does the calculation
};

#endif // SVShadingCalculationDialogH
