#ifndef SVDIALOGHYDRAULICCOMPONENTS_H
#define SVDIALOGHYDRAULICCOMPONENTS_H

#include <QDialog>
#include <QMap>
#include <QAbstractTableModel>
#include <QListWidgetItem>

#include "VICUS_Constants.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

namespace Ui {
class SVDialogHydraulicComponents;
}

namespace VICUS {
	class Network;
}

class HydraulicComponentParameterModel;

class SVDialogHydraulicComponents : public QDialog
{
	Q_OBJECT

public:
	explicit SVDialogHydraulicComponents(QWidget *parent = nullptr);
	~SVDialogHydraulicComponents();

	void edit(const unsigned int networkId);

private slots:
	void on_toolButtonAdd_clicked();

	void on_toolButtonRemove_clicked();

	void on_comboBoxNetwork_activated(int index);

	void on_lineEditName_editingFinished();

	void on_comboBoxComponentType_activated(const QString &arg1);

	void on_comboBoxHeatExchangeType_activated(const QString &arg1);

	void on_listWidget_itemClicked(QListWidgetItem *item);

private:

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

	HydraulicComponentParameterModel * m_componentParModel;

private slots:
	void on_componentParModel_editCompleted();
	void on_toolButtonCopy_clicked();
};



class HydraulicComponentItem: public QListWidgetItem
{
public:
	HydraulicComponentItem(const QString & name, QListWidget * parent = 0, int type = Type);
	unsigned id() const;
	void setId(const unsigned &id);

	QString name() const;
	void setName(const QString &name);

private:
	unsigned m_id = NANDRAD::INVALID_ID;
	QString m_name;
};



class HydraulicComponentParameterModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	HydraulicComponentParameterModel(QObject *parent = 0);

	void setComponent(const NANDRAD::HydraulicNetworkComponent & component);
	void getComponentParameter(IBK::Parameter m_para[]);
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex & index) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
private:
	NANDRAD::HydraulicNetworkComponent m_component;
	std::vector<unsigned int> m_parameterList;
signals:
	void editCompleted();

};

#endif // SVDIALOGHYDRAULICCOMPONENTS_H
