#ifndef FMUVariableTableModelH
#define FMUVariableTableModelH

#include <NANDRAD_FMIVariableDefinition.h>

#include <QAbstractTableModel>
#include <QObject>
#include <QFont>

#include <set>

class FMUVariableTableModel : public QAbstractTableModel {
	Q_OBJECT
public:

	FMUVariableTableModel(QObject *parent, bool inputVariableTable);

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;
	bool setData(const QModelIndex & index, const QVariant & value, int role) override;

	/*! This toggles use of display names in variable names. */
	void setUseDisplayNames(bool useDisplayNames);

	/*! Performs a model reset. */
	void reset();
	/*! Informs users of this model about a change of this variable (row). */
	void variableModified(unsigned int row);

	/*! Composes the model name shown in the first column. */
	QString modelName(const NANDRAD::FMIVariableDefinition & var) const;
	/*! Composes the default FMI variable name if not yet configured. */
	QString fmiVariableName(const NANDRAD::FMIVariableDefinition & var) const;

	QFont	m_itemFont;

	/*! Indicates whether this is a model for input or output variables. */
	bool	m_inputVariableTable;
	/*! If true, the model and fmi variable names are modified to show also the display name. */
	bool	m_useDisplayNames = false;
	/*! Pointer to all available (input or output) variables. */
	std::vector<NANDRAD::FMIVariableDefinition> * m_availableVariables = nullptr;

	/*! Holds information about display name substitutions for model variables. */
	struct DisplayNameSubstitution {
		/*! NANDRAD model object ID. */
		unsigned int m_objectId;
		/*! Model type string, as used in "objectref_substitutions.txt" */
		std::string m_modelType;
		/*! Display name, as used in "objectref_substitutions.txt" */
		std::string m_displayName;
	};
	/*! Reference to global display names substitution table. */
	std::vector<DisplayNameSubstitution> * m_displayNames = nullptr;
};





#endif // FMUVariableTableModelH
