#ifndef SVDBSUBNETWORKEDITWIDGET_H
#define SVDBSUBNETWORKEDITWIDGET_H

#include <QWidget>

#include "SVAbstractDatabaseEditWidget.h"

class QTableWidgetItem;

namespace Ui {
class SVDBSubNetworkEditWidget;
}

class SVDatabase;
class SVDBSubNetworkTableModel;

namespace VICUS {
	class SubNetwork;
}


class SVDBSubNetworkEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT

public:

	explicit SVDBSubNetworkEditWidget(QWidget *parent = nullptr);
	~SVDBSubNetworkEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Update widget with this. */
	void updateInput(int id) override;


private slots:

	void on_pushButtonColor_clicked();

	void on_tableWidgetElements_itemSelectionChanged();

	void on_toolButtonEditComponent_clicked();

	void on_toolButtonEditController_clicked();

	void on_lineEditSubNetworkName_editingFinished();

	void on_toolButtonAdd_clicked();

	void on_toolButtonRemove_clicked();

	void on_checkBoxElementHasHeatExchange_clicked(bool checked);

	void on_toolButtonRemoveController_clicked();

	void on_tableWidgetElements_itemChanged(QTableWidgetItem *item);

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	void updateTableWidget();

	void updateElementProperties();

	void setInletOutletIds();


	Ui::SVDBSubNetworkEditWidget			*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase								*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBSubNetworkTableModel				*m_dbModel;

	/*! Pointer to currently edited subsurface-component.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no subsurface to edit.
	*/
	VICUS::SubNetwork						*m_currentSubNet;

	int										m_currentElementIdx = -1;

};

#endif // SVDBSUBNETWORKEDITWIDGET_H
