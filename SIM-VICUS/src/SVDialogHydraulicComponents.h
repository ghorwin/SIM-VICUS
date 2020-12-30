#ifndef SVDIALOGHYDRAULICCOMPONENTS_H
#define SVDIALOGHYDRAULICCOMPONENTS_H

#include <QDialog>
#include <QMap>
#include <QListWidgetItem>

#include "VICUS_Constants.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

namespace Ui {
class SVDialogHydraulicComponents;
}

namespace VICUS {
	class Network;
}

class SVHydraulicComponentParameterModel;

class SVDialogHydraulicComponents : public QDialog
{
	Q_OBJECT

public:
	explicit SVDialogHydraulicComponents(QWidget *parent = nullptr);
	~SVDialogHydraulicComponents();

	int edit(const unsigned int networkId=0, const unsigned int currentComponentId=0);

	unsigned int currentComponentId();

private slots:
	void on_toolButtonAdd_clicked();

	void on_toolButtonRemove_clicked();

	void on_comboBoxNetwork_activated(int index);

	void on_lineEditName_editingFinished();

	void on_comboBoxComponentType_activated(const QString &arg1);

	void on_comboBoxHeatExchangeType_activated(const QString &arg1);

	void on_listWidget_itemClicked(QListWidgetItem *item);

private:

	void populateListWidget(unsigned int currentComponentId=0);

	const VICUS::Network * currentNetwork();

	void updateComponent();

	void updateTableView();

	void modifyComponent();

	void modifyTableView();

	void addComponent(const std::string & name, const NANDRAD::HydraulicNetworkComponent * comp);

	Ui::SVDialogHydraulicComponents *m_ui;

	QMap<QString, unsigned> m_mapNetworks;

	QMap<QString, unsigned> m_mapComponents;

	QMap<QString, unsigned> m_mapHeatExchangeType;

	SVHydraulicComponentParameterModel * m_componentParModel;

private slots:
	void on_componentParModel_editCompleted();
	void on_toolButtonCopy_clicked();
};



// *** HydraulicComponentItem ***

class HydraulicComponentItem: public QListWidgetItem
{
public:
	HydraulicComponentItem(const QString & name, QListWidget * parent = 0, int type = Type);
	unsigned id() const;
	void setId(const unsigned &id);
private:
	unsigned m_id = NANDRAD::INVALID_ID;
};



#endif // SVDIALOGHYDRAULICCOMPONENTS_H
