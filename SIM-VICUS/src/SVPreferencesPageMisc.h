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

#ifndef SVPreferencesPageMiscH
#define SVPreferencesPageMiscH

#include <QWidget>

namespace Ui {
	class SVPreferencesPageMisc;
}

/*! The configuration page with other settings. */
class SVPreferencesPageMisc : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(SVPreferencesPageMisc)
public:
	/*! Default constructor. */
	explicit SVPreferencesPageMisc(QWidget *parent = nullptr);
	/*! Destructor. */
	~SVPreferencesPageMisc() override;

	/*! Updates the user interface with values in Settings object.*/
	void updateUi();

signals:
	/*! Emitted, when user has modified autosave settings. */
	void autosaveSettingsChanged();

private slots:
	void on_checkBoxDontUseNativeDialogs_toggled(bool checked);

	void on_pushButtonResetDoNotShowAgainDialogs_clicked();

	void on_spinBoxAutosaveInterval_valueChanged(int value);

	void on_groupBoxAutoSaving_toggled(bool isEnabled);

private:
	Ui::SVPreferencesPageMisc *m_ui;
};


#endif // SVPreferencesPageMiscH
