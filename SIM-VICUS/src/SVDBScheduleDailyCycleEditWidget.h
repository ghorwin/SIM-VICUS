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

#ifndef SVDBScheduleDailyCycleEditWidgetH
#define SVDBScheduleDailyCycleEditWidgetH

#include <QWidget>

namespace Ui {
class SVDBScheduleDailyCycleEditWidget;
}

namespace VICUS {
class DailyCycle;
}

class SVDatabase;

class QItemDelegate;

class SVDBScheduleDailyCycleEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBScheduleDailyCycleEditWidget(QWidget *parent = nullptr);
	~SVDBScheduleDailyCycleEditWidget();

	void updateInput(VICUS::DailyCycle *dc, SVDatabase *db, bool isEditable);

private slots:
	void on_tableWidgetDayCycle_cellChanged(int row, int);

private:
	/*! Set up the modified variable of the model to true. */
	void modelModify();

	Ui::SVDBScheduleDailyCycleEditWidget *m_ui;

	/*! Cached pointer to actual daily cycle. */
	VICUS::DailyCycle					*m_dailyCycle;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Cached pointer to item delegate. */
	QItemDelegate						*m_delegate;

};

#endif // SVDBScheduleDailyCycleEditWidgetH
