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

#include <IBK_Unit.h>

#include <vector>

namespace NANDRAD {
	class OutputGrid;
}

struct SourceObject {

	SourceObject(){}

	SourceObject(unsigned int id, std::string displayName):
		m_id(id),
		m_displayName(displayName)
	{}

	/*! Indicates whether source object is set active */
	bool						m_isActive = false;

	/*! ID of Source Object */
	unsigned int				m_id;
	/*! Display Name of source object */
	std::string					m_displayName;

};

struct OutputDefinition {

	OutputDefinition(){}

	/*! Indicates whether output definition is set active */
	bool						m_isActive = false;

	/*! Type of output */
	QString						m_type;
	/*! Name of output */
	QString						m_name;
	/*! Unit of output definition */
	IBK::Unit					m_unit;
	/*! Description of output definition */
	QString						m_description;
	/*! Vector of all Vector indexes/ids */
	std::vector<unsigned int>	m_vectorIds;
	/*! Vector of all Vector Source object id(s) */
	std::vector<SourceObject>	m_sourceObjectIds;
	/*! Pointer to output grid */
	NANDRAD::OutputGrid			*m_outputGrid = nullptr;

};

class SVSimulationOutputTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	SVSimulationOutputTableModel(QObject *parent);

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;
	bool setData(const QModelIndex & index, const QVariant & value, int role) override;

	/*! Resets the table model */
	void reset();

	/*! Updated the row with changed data */
	void updateOutputData(unsigned int row);

	std::vector<OutputDefinition>			*m_outputDefinitions;

	QFont									m_itemFont;

	QString									m_fileHash;
};

#endif // SVSIMULATIONOUTPUTTABLEMODEL_H
