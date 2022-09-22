#ifndef SVPROPEDITNETWORKH
#define SVPROPEDITNETWORKH

#include <QWidget>

#include "SVUndoCommandBase.h"

namespace Ui {
class SVPropEditNetwork;
}

namespace VICUS {
class Network;
}

class SVPropEditNetwork : public QWidget {
	Q_OBJECT

public slots:

	/*! Connected to SVProjectHandler::modified(), we listen to changes in selections. */
	void onModified( int modificationType, ModificationInfo * data );

public:
	explicit SVPropEditNetwork(QWidget *parent = nullptr);
	~SVPropEditNetwork();

	/*! Updates all Ui elements and enable/disable states */
	void updateUi();

	/*! Updates network names in combobox */
	void updateComboBoxNetworks();

private slots:
	void on_pushButtonSelectFluid_clicked();

	void on_pushButtonGenerateIntersections_clicked();

	void on_pushButtonConnectBuildings_clicked();

	void on_pushButtonReduceRedundantNodes_clicked();

	void on_pushButtonReduceDeadEnds_clicked();

	void on_pushButtonRemoveSmallEdge_clicked();

	void on_pushButtonSelectPipes_clicked();

	void on_pushButtonSizePipeDimensions_clicked();

	void on_comboBoxCurrentNetwork_currentIndexChanged(int);

	void on_horizontalSliderScaleNodes_valueChanged(int value);

	void on_horizontalSliderScaleEdges_valueChanged(int value);

	void on_lineEditMaxPressureDrop_editingFinishedSuccessfully();

	void on_lineEditTemperatureSetpoint_editingFinishedSuccessfully();

	void on_lineEditTemperatureDifference_editingFinishedSuccessfully();

private:
	Ui::SVPropEditNetwork		* m_ui;

	const VICUS::Network		* m_currentNetwork = nullptr;
};

#endif // SVPROPEDITNETWORKH
