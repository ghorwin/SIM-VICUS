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


class SVPropNetworkGeometryWidget : public QWidget {
	Q_OBJECT

public slots:


public:
	explicit SVPropNetworkGeometryWidget(QWidget *parent = nullptr);
	~SVPropNetworkGeometryWidget();

	/*! Updates all Ui elements and enable/disable states */
	void updateUi();

	/*! Sets the according combobox entry and changes the current network */
	void updateCurrentNetwork();

private slots:
	void on_pushButtonSelectFluid_clicked();

	void on_pushButtonGenerateIntersections_clicked();

	void on_pushButtonConnectBuildings_clicked();

	void on_pushButtonReduceRedundantNodes_clicked();

	void on_pushButtonReduceDeadEnds_clicked();

	void on_pushButtonRemoveSmallEdge_clicked();

	void on_horizontalSliderScaleNodes_valueChanged(int value);

	void on_horizontalSliderScaleEdges_valueChanged(int value);

	void on_pushButtonSelectPipes_clicked();

	void on_lineEditMaxPressureDrop_editingFinishedSuccessfully();

	void on_lineEditTemperatureSetpoint_editingFinishedSuccessfully();

	void on_lineEditTemperatureDifference_editingFinishedSuccessfully();

	void on_pushButtonSizePipeDimensions_clicked();

	void on_pushButtonEditSimultaneity_clicked();

private:
	Ui::SVPropEditNetwork		* m_ui;

	/*! Network that is currently selected for modification */
	const VICUS::Network		* m_currentlyActiveNetwork = nullptr;

	/*! Icons for connected/disconnected network */
	QPixmap						m_iconConnected;
	QPixmap						m_iconUnconnected;
};

#endif // SVPROPEDITNETWORKH
