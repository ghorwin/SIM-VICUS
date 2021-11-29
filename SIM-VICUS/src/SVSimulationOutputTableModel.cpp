#include "SVSimulationOutputTableModel.h"

#include<QIcon>

#include <IBK_FileReader.h>
#include <IBK_StringUtils.h>

#include <NANDRAD_IDGroup.h>

SVSimulationOutputTableModel::SVSimulationOutputTableModel(QObject *parent) :
	QAbstractTableModel(parent)
{}


int SVSimulationOutputTableModel::rowCount(const QModelIndex & /*parent*/) const {
	return (int)m_variables.size();
}


int SVSimulationOutputTableModel::columnCount(const QModelIndex & /*parent*/) const {
	return 3;
}


QVariant SVSimulationOutputTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	static QStringList headers = QStringList()
		<< tr("Type")
		<< tr("Name")
		<< tr("Unit");

	if (orientation == Qt::Vertical || section < 0 || section > 2)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole :
			return headers[section];
	}
	return QVariant();
}


QVariant SVSimulationOutputTableModel::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	const OutputVariable & var = m_variables[(size_t)index.row()];
	switch (role) {
		case Qt::DisplayRole : {
			switch (index.column()) {
				case 0 : // type name
					return QString::fromStdString(var.m_objectTypeName);
				case 1 : // quantity
					return QString::fromStdString(var.m_quantity);
				case 2 : // unit
					return QString::fromStdString(var.m_unit);
			}
		} break;
	}
	return QVariant();
}


void SVSimulationOutputTableModel::updateListFromFile(const QString & outputRefListFilepath) {
	FUNCID(SVSimulationOutputTableModel::updateListFromFile);


	// we parse a file with the following structure

	//	Variable name                                     	Source object id(s) 	Vector indexes/ids  	Unit      	Description
	//	ConstructionInstance.ElementTemperature           	101                 	0,1,2               	C         	Finite-volume mean element temperature
	//	ConstructionInstance.FluxHeatConductionA          	101                 	                    	W         	Heat conduction flux across interface A (into construction)
	//	ConstructionInstance.FluxHeatConductionB          	101                 	                    	W         	Heat conduction flux across interface B (into construction)

	try {
		IBK::Path varFile(outputRefListFilepath.toStdString());
		// missing file?
		if (!varFile.exists()) {
			// now process line-by-line
			beginResetModel();
			m_variables.clear();
			endResetModel();
			return;
		}
		std::vector<std::string> lines;
		IBK::FileReader::readAll(varFile, lines, std::vector<std::string>());

		// now process line-by-line
		beginResetModel();
		m_variables.clear();

		std::vector<std::string> tokens;
		for (unsigned int i=0; i<lines.size(); ++i) {
			// skip header line
			if (i == 0)
				continue;

			const std::string &line = lines[i];
			// skip empty (trailing) lines
			if (line.find_first_not_of(" \t\r") == std::string::npos)
				continue;

			// split columns
			IBK::explode(line, tokens, "\t", IBK::EF_NoFlags);

			OutputVariable var;
			if (tokens.size() != 5)
				throw IBK::Exception(IBK::FormatString("Invalid format in line '%1'.").arg(line), FUNC_ID);

			// first column contains encoded object-type.variable name
			std::string::size_type pos = tokens[0].find(".");
			if (pos == std::string::npos)
				throw IBK::Exception(IBK::FormatString("Invalid format of variable ref '%1'.").arg(tokens[0]), FUNC_ID);

			var.m_objectTypeName = tokens[0].substr(0, pos);
			var.m_quantity = tokens[0].substr(pos+1);

			if (tokens[1].find_first_not_of(" ") == std::string::npos)
				throw IBK::Exception(IBK::FormatString("Missing object IDs in line '%1'.").arg(line), FUNC_ID);

			// split the object IDs into numbers
			try {
				NANDRAD::IDGroup ids;
				ids.setEncodedString(tokens[1]);
				var.m_objectIds = ids.m_ids;
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Invalid object ID format in line '%1'.").arg(line), FUNC_ID);
			}

			// vector IDs are optional
			if (tokens[2].find_first_not_of(" ") != std::string::npos) {
				try {
					NANDRAD::IDGroup ids;
					ids.setEncodedString(tokens[2]);
					var.m_vectorIds = ids.m_ids;
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Invalid vector ID format in line '%1'.").arg(line), FUNC_ID);
				}
			}

			// unit
			var.m_unit = tokens[3];

			m_variables.push_back(var);
		}

		endResetModel();
	}
	catch (IBK::Exception &ex) {
		m_variables.clear(); // clear the list of variables again
		endResetModel();
		throw IBK::Exception(ex, IBK::FormatString("Could not open file '%1' with output definitons.").arg(outputRefListFilepath.toStdString()), FUNC_ID);
	}
}

