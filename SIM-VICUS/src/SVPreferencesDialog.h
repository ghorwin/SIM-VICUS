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

#ifndef SVPreferencesDialogH
#define SVPreferencesDialogH

#include <QDialog>

class SVPreferencesPageTools;
class SVPreferencesPageStyle;
class SVPreferencesPageMisc;

namespace Ui {
	class SVPreferencesDialog;
}

/*! Implementation of the preferences dialog. */
class SVPreferencesDialog : public QWidget {
	Q_OBJECT
public:
	/*! Constructor.*/
	SVPreferencesDialog(QWidget * parent);
	/*! Destructor. */
	~SVPreferencesDialog() override;

	/*! Spawns the dialog and returns when user has closed the dialog.
		\param initialPage The index of the page to be shown initially.
	*/
	void edit(int initialPage = 0);

	/*! Provides read-only access to pageStyle() so that signals can be connected. */
	const SVPreferencesPageStyle * pageStyle() const { return m_pageStyle; }

signals:
	/*! Emitted to signal a preference decision for the calling window/dialog event loop.*/
	void closed();

protected:
	void closeEvent(QCloseEvent *event) override;

private:
	/*! Transfers values from Settings object to user interface (config pages).*/
	void updateUi();

	/*! GUI member. */
	Ui::SVPreferencesDialog			*m_ui;

	/*! The Tools page. */
	SVPreferencesPageTools			*m_pageTools;
	/*! The Style page. */
	SVPreferencesPageStyle			*m_pageStyle;
	/*! The Misc page. */
	SVPreferencesPageMisc			*m_pageMisc;
};

#endif // SVPreferencesDialogH
