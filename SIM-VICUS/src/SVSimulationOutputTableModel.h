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

#include <QAbstractTableModel>

#include <set>
#include <vector>

namespace VICUS {
	class OutputDefinition;
}


/*! This table model shows the available outputs from output_reference_list.txt.
	Qt::DisplayRole returns the translated object name with keyword in parenthesis.

	Custom data roles are:

	- Qt::UserRole  returns a set of object IDs
	- Qt::UserRole+1 returns the set of vector IDs
	- Qt::UserRole+2 returns the object type string as keyword
*/
class SVSimulationOutputTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	SVSimulationOutputTableModel(QObject *parent);

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	QVariant data(const QModelIndex & index, int role) const override;

	/*! Updates the internal data storage of the model. */
	void updateListFromFile(const QString & outputRefListFilepath);

	/*! Checks if the defined output is available. */
	bool haveOutput(const VICUS::OutputDefinition& of) const;

private:

	/*! Holds the data of a single line in the file 'output_reference_list.txt'. */
	struct OutputVariable {
		std::string m_objectTypeName;
		std::string m_quantity;
		std::string m_description;
		std::string m_unit;
		std::set<unsigned int> m_objectIds;
		std::set<unsigned int> m_vectorIds;
	};

	/*! List of variables from output definitions file. */
	std::vector<OutputVariable> m_variables;
};

Q_DECLARE_METATYPE(std::set<unsigned int>)

#endif // SVSimulationOutputTableModelH
