#ifndef SVSimulationStartNetworkSimH
#define SVSimulationStartNetworkSimH

#include <QDialog>

namespace Ui {
class SVSimulationStartNetworkSim;
}

/*! Dialog for starting a simulation. */
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

	Ui::SVSimulationStartNetworkSim		*m_ui;

	QStringList							m_cmdLine;
};

#endif // SVSimulationStartNetworkSimH
