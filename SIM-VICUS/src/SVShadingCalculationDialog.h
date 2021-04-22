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

	enum OutputType {
		TsvFile,		///< write output to tsv file
		D6oFile,		///< write output to d6o file
		D6bFile			///< write output to d6b file
	};

	int edit();

public slots:
	void evaluateResults();

private slots:
	void on_pushButtonCalculate_clicked();

	void on_lineEditStartDate_editingFinished();

	void on_lineEditEndDate_editingFinished();

	void on_lineEditDuration_editingFinishedSuccessfully();

	void on_lineEditGridSize_editingFinished();

	void on_lineEditSunCone_editingFinished();

	void on_comboBoxFileType_currentIndexChanged(int index);

private:
	void								updateTimeFrameEdits();


	Ui::SVShadingCalculationDialog		*m_ui;

	const NANDRAD::SimulationParameter	*m_simulationParameter;
	const SVSimulationStartNandrad		*m_simStartWidget;

	double								m_gridSize = 0.1;
	double								m_sunCone = 3;

	OutputType							m_outputType = OutputType::TsvFile;

	std::vector<const VICUS::Surface*>	m_selSurfaces;
	std::vector<const VICUS::Surface*>	m_selObstacles;

	/*! Local copy of our project data, modified in dialog and synced with global
		project when dialog is confirmed.
	*/
	VICUS::Project						m_localProject;

	SH::StructuralShading				m_shading;
};

#endif // SVShadingCalculationDialogH
