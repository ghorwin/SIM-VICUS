/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVDatabaseEditDialogH
#define SVDatabaseEditDialogH

#include <QDialog>

#include <VICUS_InternalLoad.h>

namespace Ui {
	class SVDatabaseEditDialog;
}

class SVAbstractDatabaseTableModel;
class SVAbstractDatabaseEditWidget;
//class SVInternalLoadsTableModel;
class QModelIndex;
class QGroupBox;
class QSortFilterProxyModel;


#include <VICUS_Constants.h>

/*! The generic edit dialog. */
class SVDatabaseEditDialog : public QDialog {
	Q_OBJECT

public:
	/*! Constructor of the dialog.
		The dialog is being configured with the arguments.

		\note Passing an empty editWidgetTitle will not generate a groupbox, but a plain widget instead.
	*/
	explicit SVDatabaseEditDialog(QWidget *parent, SVAbstractDatabaseTableModel * tableModel,
								  SVAbstractDatabaseEditWidget * editWidget,
								  const QString & title, const QString & editWidgetTitle,
								  bool horizontalLayout);
	~SVDatabaseEditDialog() override;

	/*! Starts the dialog in "edit" mode. */
	void edit(unsigned int initialId = VICUS::INVALID_ID);

	/*! Starts the dialog in "select mode".
		\param initialId The item indicated by this ID is initially selected.
		\return If an item was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected item. Otherwise, if the
				dialog was aborted, the function returns VICUS::INVALID_ID.
	*/
	unsigned int select(unsigned int initialId, bool resetModel = true, QString filterText = "", int filterColumn = -1);

	/*! Event filter for resizing events in order to resize row to its contents. */
	bool eventFilter(QObject * obj, QEvent * event) override;

	SVAbstractDatabaseTableModel * dbModel() const;

private slots:
	void on_pushButtonSelect_clicked();
	void on_pushButtonCancel_clicked();
	void on_pushButtonClose_clicked();

	void on_toolButtonAdd_clicked();
	void on_toolButtonCopy_clicked();
	void on_toolButtonRemove_clicked();

	/*! Connected to the respective signal in the table view.
		Enables/disables the remove button.
	*/
	void onCurrentIndexChanged(const QModelIndex &current, const QModelIndex &/*previous*/);

	void on_pushButtonReloadUserDB_clicked();

	void on_tableView_doubleClicked(const QModelIndex &index);

	void on_toolButtonRemoveFromUserDB_clicked();

	void on_toolButtonStoreInUserDB_clicked();

	void on_pushButtonRemoveUnusedElements_clicked();

	void onStyleChanged();

	void on_toolButtonApplyFilter_clicked();

	void on_comboBoxColumn_currentIndexChanged(int /*index*/);

	void on_lineEditFilter_returnPressed();

private:
	/*! If table contains an element with matching ID, this row is made current.
		Signals are blocked in this function.
	*/
	void selectItemById(unsigned int id);
	/*! Writes the user DB in case it was modified
	 */
	void writeUserDB();


	// Factory functions to create all the individual dialogs
	static SVDatabaseEditDialog * createMaterialEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createConstructionEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createComponentEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createSubSurfaceComponentEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createWindowEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createWindowGlazingSystemEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createBoundaryConditionsEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createScheduleEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createInternalLoadsEditDialog(QWidget * parent, VICUS::InternalLoad::Category category);
	static SVDatabaseEditDialog * createZoneControlThermostatEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createZoneControlVentilationNaturalEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createZoneControlShadingEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createZoneIdealHeatingCoolingEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createInfiltrationEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createSurfaceHeatingSystemEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createVentilationNaturalEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createSupplySystemsEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createNetworkComponentEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createPipeEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createFluidEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createNetworkControllerEditDialog(QWidget * parent);
	static SVDatabaseEditDialog * createSubNetworkEditDialog(QWidget * parent);

	static void resizeDBDialog(QDialog *dlg);

	Ui::SVDatabaseEditDialog *m_ui;

	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;
	/*! The table model (owned). */
	SVAbstractDatabaseTableModel	*m_dbModel		= nullptr;
	/*! The edit widget (owned). */
	SVAbstractDatabaseEditWidget	*m_editWidget	= nullptr;
	QWidget							*m_editWidgetContainerWidget = nullptr;

	QString							m_currentFilter = "";

	friend class SVMainWindow;
};

#endif // SVDatabaseEditDialogH
