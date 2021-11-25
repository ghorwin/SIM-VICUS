#ifndef SVSIMULATIONETWORKOPTIONS_H
#define SVSIMULATIONETWORKOPTIONS_H

#include <QWidget>

#include <VICUS_Network.h>

namespace Ui {
class SVSimulationNetworkOptions;
}


/*! Option page for network simulation. */
class SVSimulationNetworkOptions : public QWidget {
	Q_OBJECT

public:
	explicit SVSimulationNetworkOptions(QWidget *parent, std::vector<VICUS::Network> &networks);
	~SVSimulationNetworkOptions();

	void updateUi();

private slots:
	void on_comboBoxNetwork_activated(int index);
	void on_comboBoxModelType_activated(int index);
	void on_lineEditDefaultFluidTemperature_editingFinished();
	void on_lineEditReferencePressure_editingFinished();
	void on_lineEditMaxPipeDiscretization_editingFinished();

	void on_lineEditPipeSpacing_editingFinished();

	void on_lineEditPipeDepth_editingFinished();

	void on_lineEditNumberOfSoilModels_editingFinished();

private:
	Ui::SVSimulationNetworkOptions *m_ui;

	/*! Pointer to local project's network data. */
	std::vector<VICUS::Network>		*m_networks;

	VICUS::Network					*m_current = nullptr;
};

#endif // SVSIMULATIONETWORKOPTIONS_H
