#ifndef SVPROPNETWORKHEATEXCHANGEWIDGETH
#define SVPROPNETWORKHEATEXCHANGEWIDGETH

#include <QWidget>

namespace Ui {
class SVPropNetworkHeatExchangeWidget;
}

class SVPropNetworkEditWidget;

class SVPropNetworkHeatExchangeWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropNetworkHeatExchangeWidget(QWidget *parent = nullptr);
	~SVPropNetworkHeatExchangeWidget();

	void updateUi();

	void updateTableWidget();

	void clearUi();

	void setWidgetsEnabled(bool enabled);

	/*! Modifies the selected nodes/edges heat echange parameters in the project */
	void modifyHeatExchangeParameters();

private slots:
	void on_comboBoxHeatExchangeType_activated(int index);

	void on_lineEditHeatFlux_editingFinishedSuccessfully();

	void on_lineEditTemperature_editingFinishedSuccessfully();

	void on_lineEditHXTransferCoefficient_editingFinishedSuccessfully();

	void on_widgetHeatExchangeSplineFile_editingFinished();

	void on_toolButtonHeatExchangeSpline_clicked();

private:
	Ui::SVPropNetworkHeatExchangeWidget *m_ui;

	SVPropNetworkEditWidget				*m_pa;

};

#endif // SVPROPNETWORKHEATEXCHANGEWIDGETH
