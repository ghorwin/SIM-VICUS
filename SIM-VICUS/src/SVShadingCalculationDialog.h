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

class SVShadingCalculationDialog : public QDialog
{


	Q_OBJECT

public:
	explicit SVShadingCalculationDialog(QWidget *parent = nullptr);
	~SVShadingCalculationDialog();

	int edit();

public slots:
	void setProgressBar(double progress);
	void evaluateResults();
	void stopCalculation();

private slots:


	void on_pushButtonCalculate_clicked();


	void on_lineEditStartDate_editingFinished();
	void on_lineEditEndDate_editingFinished();
	void on_lineEditDuration_editingFinishedSuccessfully();

	void on_lineEditGridSize_editingFinished();

	void on_lineEditSunCone_editingFinished();

	void on_pushButtonChancel_clicked();

private:
	Ui::SVShadingCalculationDialog		*m_ui;

	void								updateTimeFrameEdits();

	const NANDRAD::SimulationParameter	*m_simulationParameter;
	const SVSimulationStartNandrad		*m_simStartWidget;

	double								m_gridSize = 0.1;
	double								m_sunCone = 3;

	std::vector<const VICUS::Surface*>	m_selSurfaces;
	std::vector<const VICUS::Surface*>	m_selObstacles;

	QProgressDialog						*m_progressDialog = nullptr;

	QTimer								*m_timer = nullptr;
	QElapsedTimer						*m_elapsedTime = nullptr;

	/*! Local copy of our project data, modified in dialog and synced with global
		project when dialog is confirmed.
		This is also the cache for data edited in this dialog in the various edit pages.
	*/
	VICUS::Project						m_localProject;

	SH::StructuralShading				m_shading;

};

#endif // SVShadingCalculationDialogH
