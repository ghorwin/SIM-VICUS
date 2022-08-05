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

#ifndef SVDBZoneTemplateEditDialogH
#define SVDBZoneTemplateEditDialogH

#include <QDialog>

namespace Ui {
	class SVDBZoneTemplateEditDialog;
}

class QModelIndex;
class QGroupBox;
class QSortFilterProxyModel;
class SVDBZoneTemplateTreeModel;
class SVDBZoneTemplateEditWidget;

#include <VICUS_Constants.h>
#include <VICUS_ZoneTemplate.h>

/*! The edit dialog for materials types. */
class SVDBZoneTemplateEditDialog : public QDialog {
	Q_OBJECT

public:
	/*! Constructor of the dialog.
		The dialog is being configured with the arguments.

		\note Passing an empty editWidgetTitle will not generate a groupbox, but a plain widget instead.
	*/
	explicit SVDBZoneTemplateEditDialog(QWidget *parent);
	~SVDBZoneTemplateEditDialog() override;

	/*! Starts the dialog in "edit" mode. */
	void edit(unsigned int initialId = VICUS::INVALID_ID);

	/*! Starts the dialog in "select mode".
		\param initialId The item indicated by this ID is initially selected.
		\return If an item was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected item. Otherwise, if the
				dialog was aborted, the function returns VICUS::INVALID_ID.
	*/
	unsigned int select(unsigned int initialId);


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

	void on_treeView_doubleClicked(const QModelIndex &index);

	void onSelectSubTemplate(unsigned int zoneTemplateID, VICUS::ZoneTemplate::SubTemplateType subTemplateType);

	void on_treeView_expanded(const QModelIndex &index);

	void on_treeView_collapsed(const QModelIndex &index);

private:
	/*! If table contains an element with matching ID, this row is made current.
		Signals are blocked in this function.
	*/
	void selectItemById(unsigned int id);

	Ui::SVDBZoneTemplateEditDialog *m_ui;

	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;
	/*! The table model (owned). */
	SVDBZoneTemplateTreeModel		*m_dbModel		= nullptr;
	/*! The edit widget (owned). */
	SVDBZoneTemplateEditWidget		*m_editWidget	= nullptr;

	QGroupBox						*m_groupBox = nullptr;

	friend class SVMainWindow;

};

#endif // SVDBZoneTemplateEditDialogH
