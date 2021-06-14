#include "VariableTableModel.h"

#include <QColor>
#include <QFile>

VariableTableModel::VariableTableModel(QObject *parent, bool inputVars)
	: QAbstractTableModel(parent), m_inputVars(inputVars)
{
}


QVariant VariableTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal) {
		if( role == Qt::DisplayRole) {
			switch(section)
			{
				case C_VariableName: return tr("NANDRAD Variable Name");
				case C_ObjectId: return tr("Object ID");
				case C_VectorValueIndexId: return tr("Vector value index/ID");
				case C_Unit: return tr("Unit");
				case C_FMIVariableName: return tr("FMI Variable Name");
				case C_FMIValueReference: return tr("FMI value reference");
				case C_FMIType: return tr("FMI Type");
				case C_Description: return tr("Description");
			}
		}
		else if (role == Qt::DisplayRole)
			return QString("%1").arg(section + 1);
	}
	return QVariant();
}


int VariableTableModel::rowCount(const QModelIndex &/*parent*/) const {
	return m_variables.size();
}


int VariableTableModel::columnCount(const QModelIndex &parent) const {
	if (parent.isValid())
		return 0;
	return NUM_Columns;
}


QVariant VariableTableModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (!index.isValid() || index.row()<0 || index.row() >= static_cast<int>(m_variables.size()))
		return QVariant();

	int column(index.column());
	const NANDRAD::FMIVariableDefinition& inputVar = m_variables[(unsigned int) index.row()];

	switch(role) {
		case Qt::DisplayRole:
		case Qt::EditRole: {
			switch(column) {
				case C_VariableName: return QString(inputVar.m_fmiVarName.c_str());
				case C_ObjectId: return inputVar.m_objectId;
				case C_VectorValueIndexId: {
					if(inputVar.m_vectorIndex == NANDRAD::INVALID_ID)
						return QVariant();
					return inputVar.m_vectorIndex;
				}
				case C_Unit: return QString(inputVar.m_unit.c_str());
				case C_FMIVariableName: return QString(inputVar.m_fmiVarName.c_str());
				case C_FMIValueReference: {
					if(inputVar.m_fmiValueRef == NANDRAD::INVALID_ID)
						return QVariant();
					return inputVar.m_fmiValueRef;
				}
				case C_FMIType: return QString(inputVar.m_fmiTypeName.c_str());
				case C_Description: return QString(inputVar.m_fmiVarDescription.c_str());
				default: return QVariant();
			}
		}
		case Qt::ForegroundRole: {
			// not used values are marked with invalid fmi id
			if(inputVar.m_fmiValueRef == NANDRAD::INVALID_ID)
				return QColor(Qt::gray);
			return QColor(Qt::black);
		}
		default: return QVariant();
	}
}


bool VariableTableModel::updateVariableLists(const QString & nandradProjectFilePath, QString & errmsg) {
	// now parse the variable lists
	IBK::Path varDir(nandradProjectFilePath.toStdString());
	varDir = varDir.withoutExtension() / "var";

	m_variables.clear();
	QString inputVarsFile = QString::fromStdString( (varDir / "input_reference_list.txt").str() );
	// an error occured-> show message
	if (!parseVariableList(inputVarsFile, errmsg) )
		return false;

	beginResetModel();
	endResetModel();
	return true;
}


bool VariableTableModel::parseVariableList(const QString & varsFile, QString &errmsg) {
	beginResetModel();
	bool result = parseVariableFile(varsFile, errmsg);
	endResetModel();
	return result;
}


bool VariableTableModel::parseVariableFile(const QString & varsFile, QString &errmsg) {
	m_variables.clear();
	QFile inputVarF(varsFile);
	if (!inputVarF.open(QFile::ReadOnly)) {
			errmsg = QString("Could not read file '%1'. Re-run solver initialization!")
								  .arg(varsFile);
		return false;
	}

	QStringList vars = QString(inputVarF.readAll()).split('\n');

	//	Parse variable definitions as in the following lines:
	//
	//	Model.VentilationHeatFlux                         	1                   	1,2,3
	//	Zone.AirTemperature                               	1,2,3
	//
	// or in the output ref list file
	//
	//	Model.VentilationHeatFlux                         	1                   	2,3                 	W         	Natural ventilation/infiltration heat flux
	//	Zone.AirTemperature                               	1,2,3               	                    	C         	Room air temperature.

	// we process all but first line
	for (int j=1; j<vars.count(); ++j) {
		if (vars[j].trimmed().isEmpty())
			continue; // skip (trailing) empty lines)
		// Note: vars[j] must not be trimmed before calling split, since we may have several trailing \t which are important!
		QStringList tokens = vars[j].split('\t');
		if (tokens.count() < 3) {
			errmsg = QString("Invalid data in file '%1'. Re-run solver initialization!")
				.arg(varsFile);
			return false;
		}

		// extract all the data we need from the strings
		QStringList varNameTokens = tokens[0].trimmed().split(".");
		if (varNameTokens.count() != 2) {
			errmsg = QString("Invalid data in file '%1'. Malformed variable name '%2'. Re-run solver initialization!")
				.arg(varsFile).arg(tokens[0]);
			return false;
		}
		QString objTypeName = varNameTokens[0];			// "Zone"
		QString nandradVarName = varNameTokens[1];		// "AirTemperature"
		QString unit;
		if (tokens.count() > 3) {
			unit = tokens[3].trimmed();
			try {
				// convert to base SI unit, except for some commonly used units without conversion factor to base unit
				if (unit != "W" &&
					unit != "W/m2" &&
					unit != "W/m3")
				{
					IBK::Unit u(unit.toStdString());
					unit = QString::fromStdString(u.base_unit().name());
				}
			}
			catch (...) {
				errmsg = QString("Invalid data in file '%1'. Unrecognized unit '%2'. Re-run solver initialization!")
					.arg(varsFile).arg(unit);
				return false;
			}
		}
		QString description;
		if (tokens.count() > 4)
			description = tokens[4].trimmed();


		// split object IDs and vector-value IDs
		std::vector<unsigned int>	m_objectIds;
		std::vector<unsigned int>	m_vectorIndexes;
		QString idString = tokens[1].trimmed();
		if (idString.isEmpty()) {
			errmsg = QString("Invalid data in file '%1'. Object ID required for variable '%2'. Re-run solver initialization!")
				.arg(varsFile).arg(tokens[0]);
			return false;
		}
		QStringList ids = idString.split(",");
		for (QString idstr : ids)
			m_objectIds.push_back( idstr.toUInt());


		idString = tokens[2].trimmed();
		if (!idString.isEmpty()) { // empty column with vector indexes is ok for scalar results
			QStringList ids = idString.split(",");
			for (QString idstr : ids)
				m_vectorIndexes.push_back( idstr.toUInt());
		}

		// generate a variable for each combination of object ID and vector reference
		for (unsigned int objID : m_objectIds) {

			if (m_vectorIndexes.empty()) {
				NANDRAD::FMIVariableDefinition varDef;
				varDef.m_varName = tokens[0].trimmed().toStdString(); // "Zone.AirTemperature"
				// compose a variable name
				varDef.m_fmiVarName = QString("%1(%2).%3")
						.arg(objTypeName).arg(objID).arg(nandradVarName)
						.toStdString();
				varDef.m_objectId = objID;
				varDef.m_vectorIndex = NANDRAD::INVALID_ID;
				varDef.m_fmiTypeName = ""; // TODO : how to determine the correct type?
				varDef.m_unit = unit.toStdString();
				varDef.m_fmiVarDescription = description.toStdString();
				varDef.m_fmiValueRef = NANDRAD::INVALID_ID; // will be set from either existing var in project or when configured
				// set default start value
				varDef.m_fmiStartValue = 0;
				if (varDef.m_unit == "K")				varDef.m_fmiStartValue = 293.15;
				else if (varDef.m_unit == "Pa")			varDef.m_fmiStartValue = 101325;

				m_variables.push_back(varDef);
			}
			else {
				for (unsigned int vecIdx : m_vectorIndexes) {
					NANDRAD::FMIVariableDefinition varDef;
					varDef.m_varName = tokens[0].trimmed().toStdString(); // "Zone.AirTemperature"
					varDef.m_fmiVarName = QString("%1(%2).%3(%4)")
							.arg(objTypeName).arg(objID).arg(nandradVarName).arg(vecIdx)
							.toStdString();
					varDef.m_objectId = objID;
					varDef.m_vectorIndex = vecIdx;
					varDef.m_fmiTypeName = ""; // TODO : how to determine the correct type?
					varDef.m_unit = unit.toStdString();
					varDef.m_fmiVarDescription = description.toStdString();
					varDef.m_fmiValueRef = NANDRAD::INVALID_ID; // will be set from either existing var in project or when configured
					// set default start value
					varDef.m_fmiStartValue = 0;
					if (varDef.m_unit == "K")				varDef.m_fmiStartValue = 293.15;
					else if (varDef.m_unit == "Pa")			varDef.m_fmiStartValue = 101325;

					m_variables.push_back(varDef);
				}

			}
		}
	}
	return true;
}


