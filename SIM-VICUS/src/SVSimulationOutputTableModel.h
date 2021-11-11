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

#ifndef SVSimulationOutputTableModelH
#define SVSimulationOutputTableModelH

#include <QObject>
#include <QAbstractTableModel>
#include <QCryptographicHash>
#include <QFont>
#include <QColor>

#include <VICUS_OutputDefinition.h>

#include <IBK_Unit.h>

#include <vector>

namespace NANDRAD {
	class OutputGrid;
}

/*! This table model shows the available outputs. */
class SVSimulationOutputTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	SVSimulationOutputTableModel(QObject *parent);

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

	/*! Resets the table model */
	void reset();

	/*! Updated the row with changed data */
	void updateOutputData(unsigned int row);

	std::vector<VICUS::OutputDefinition>						*m_outputDefinitions = nullptr;

	QFont														m_itemFont;
};

#endif // SVSIMULATIONOUTPUTTABLEMODEL_H
