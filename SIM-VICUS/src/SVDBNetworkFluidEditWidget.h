#ifndef SVDBNETWORKFLUIDEDITWIDGET_H
#define SVDBNETWORKFLUIDEDITWIDGET_H

#include "SVAbstractDatabaseEditWidget.h"

namespace Ui {
class SVDBNetworkFluidEditWidget;
}

namespace VICUS {
	class NetworkFluid;
}

class SVDBNetworkFluidTableModel;
class SVDatabase;


class SVDBNetworkFluidEditWidget :  public SVAbstractDatabaseEditWidget {

	Q_OBJECT

public:

	explicit SVDBNetworkFluidEditWidget(QWidget *parent = nullptr);
	~SVDBNetworkFluidEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;

private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonComponentColor_colorChanged();

	void on_lineEditDensity_editingFinished();
	void on_lineEditHeatCapacity_editingFinished();
	void on_lineEditThermalConductivity_editingFinished();

private:
	Ui::SVDBNetworkFluidEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBNetworkFluidTableModel			*m_dbModel;

	/*! Pointer to currently edited component.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no component to edit.
	*/
	VICUS::NetworkFluid					*m_currentFluid;
};


#endif // SVDBNETWORKFLUIDEDITWIDGET_H
