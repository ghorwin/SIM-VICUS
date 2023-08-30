#ifndef SVSIMULATIONETWORKOPTIONS_H
#define SVSIMULATIONETWORKOPTIONS_H

#include <QWidget>

#include <VICUS_Network.h>

class ModificationInfo;

namespace Ui {
class SVSimulationNetworkOptions;
}


/*! Option page for network simulation. */
class SVSimulationNetworkOptions : public QWidget {
	Q_OBJECT

public:
	explicit SVSimulationNetworkOptions(QWidget *parent);
	~SVSimulationNetworkOptions();

	void updateUi();

public slots:
	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * /*data*/ );

private slots:
	void on_comboBoxModelType_activated(int index);
	void on_lineEditDefaultFluidTemperature_editingFinished();
	void on_lineEditReferencePressure_editingFinished();
	void on_lineEditMaxPipeDiscretization_editingFinished();

	void on_lineEditPipeSpacing_editingFinished();

	void on_lineEditPipeDepth_editingFinished();

	void on_lineEditNumberOfSoilModels_editingFinished();

	void on_groupBoxHeatExchangeWithGround_clicked(bool checked);

	void on_comboBoxNetwork_activated(int index);

private:

	void updateNetworkParameters();

	Ui::SVSimulationNetworkOptions *m_ui;
};

#endif // SVSIMULATIONETWORKOPTIONS_H
