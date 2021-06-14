#ifndef VariableTableModelH
#define VariableTableModelH

#include <QAbstractTableModel>

#include <NANDRAD_Project.h>


/*! A table model to show a list of FMI variables in a table view.
	The same model is used for input and output variables.
	Flag m_inputVars indicates, that this is indeed a table model for input variables (determines model behaviour).
*/
class VariableTableModel : public QAbstractTableModel {
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

	explicit VariableTableModel(QObject *parent = nullptr, bool inputVars = true);

	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	/*! Parses the input files, then updates m_availableInputVariables.*/
	bool updateVariableLists(const QString &nandradProjectFilePath, QString &errmsg);

	/*! Reads a variable definition file and generates a map with model variables versus object/vector IDs. */
	bool parseVariableList(const QString & varsFile, QString &errmsg);

	/*! Grant read-only access to model data. */
	const std::vector<NANDRAD::FMIVariableDefinition> & variables() const { return m_variables; }

private:
	bool parseVariableFile(const QString & varsFile, QString &errmsg);

	bool											m_inputVars;
	/*! Holds all _available_ variable definitions, that are managed by this table model. */
	std::vector<NANDRAD::FMIVariableDefinition>		m_variables;


};

#endif // INPUTVARIABLESTABLEMODEL_H
