#ifndef SVPROPNETWORKSUBSTATIONWIDGET_H
#define SVPROPNETWORKSUBSTATIONWIDGET_H

#include <QWidget>

class QTableWidgetItem;

namespace Ui {
class SVPropNetworkSubStationWidget;
}

class SVPropNetworkEditWidget;

class SVPropNetworkSubStationWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropNetworkSubStationWidget(QWidget *parent = nullptr);
	~SVPropNetworkSubStationWidget();

	void updateUi();

	void updateTableWidget();

	void clearUi();

	void setWidgetsEnabled(bool enabled);

private slots:
	void on_pushButtonEditSubNetworks_clicked();

	void on_pushButtonExchangeSubNetwork_clicked();

	void on_pushButtonSelectNodesWithSubNetwork_clicked();

	void on_pushButtonAssignSubNetwork_clicked();

	void on_tableWidgetSubNetworks_itemSelectionChanged();

	void on_tableWidgetSubNetworks_itemDoubleClicked(QTableWidgetItem */*item*/);

private:
	Ui::SVPropNetworkSubStationWidget	*m_ui;

	SVPropNetworkEditWidget				*m_pa;
};

#endif // SVPROPNETWORKSUBSTATIONWIDGET_H
