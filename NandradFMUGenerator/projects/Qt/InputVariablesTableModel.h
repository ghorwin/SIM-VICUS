#ifndef INPUTVARIABLESTABLEMODEL_H
#define INPUTVARIABLESTABLEMODEL_H

#include <QAbstractTableModel>

#include <NANDRAD_Project.h>


class InputVariablesTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum Columns {
		C_VariableName,
		C_ObjectId,
		C_VectorValueIndexId,
		C_Unit,
		C_FMIVariableName,
		C_FMIValueReference,
		C_FMIType,
		C_Description,
		NUM_Columns
	};

	explicit InputVariablesTableModel(QObject *parent = nullptr);

	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	/*! Parses the input files, then updates m_availableInputVariables.*/
	bool updateVariableLists(const QString &nandradProjectFilePath, QString &errmsg);

	/*! Reads a variable definition file and generates a map with model variables versus object/vector IDs. */
	bool parseVariableList(const QString & varsFile, std::vector<NANDRAD::FMIVariableDefinition> & modelVariables, QString &errmsg);

private:
	/*! Holds all _available_ input variable definitions. */
	std::vector<NANDRAD::FMIVariableDefinition>		m_availableInputVariables;

};

#endif // INPUTVARIABLESTABLEMODEL_H
