#ifndef HYDRAULICCOMPONENTPARAMETERMODEL_H
#define HYDRAULICCOMPONENTPARAMETERMODEL_H

#include <QAbstractTableModel>

#include <NANDRAD_HydraulicNetworkComponent.h>


class SVHydraulicComponentParameterModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	SVHydraulicComponentParameterModel(QObject *parent = 0);

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
#endif // HYDRAULICCOMPONENTPARAMETERMODEL_H
