#ifndef SVSimulationStartNetworkSimH
#define SVSimulationStartNetworkSimH

#include <QDialog>
#include <QMap>

namespace Ui {
	class SVSimulationStartNetworkSim;
}

namespace NANDRAD {
	class Project;
}

/*! Dialog for starting a simulation.
	It generates a NANDRAD project based on the current VICUS project contents.
*/
class SVSimulationStartNetworkSim : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationStartNetworkSim(QWidget *parent = nullptr);
	~SVSimulationStartNetworkSim();

	/*! Starts the dialog and populates its content based on the current project's content. */
	void edit();

private slots:
	void on_checkBoxCloseConsoleWindow_toggled(bool checked);

	void on_pushButtonRun_clicked();

private:
	void updateCmdLine();

	bool generateNandradProject(NANDRAD::Project & p) const;

	Ui::SVSimulationStartNetworkSim		*m_ui;

	QMap<QString, unsigned int>			m_networksMap;
	QString								m_solverExecutable;
	QStringList							m_cmdLine;
	QString								m_targetProjectFile;
};

#endif // SVSimulationStartNetworkSimH
