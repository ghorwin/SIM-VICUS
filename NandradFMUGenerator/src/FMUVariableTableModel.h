#ifndef FMUVariableTableModelH
#define FMUVariableTableModelH

#include <NANDRAD_FMIVariableDefinition.h>

#include <QAbstractTableModel>
#include <QObject>
#include <QFont>

class FMUVariableTableModel : public QAbstractTableModel {
	Q_OBJECT
public:

	FMUVariableTableModel(QObject *parent, bool inputVariableTable);

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	/*! Performs a model reset. */
	void reset();
	/*! Informs users of this model about a change of this variable (row). */
	void variableModified(unsigned int row);

	QFont	m_itemFont;

	bool	m_inputVariableTable;
	std::vector<NANDRAD::FMIVariableDefinition> * m_availableVariables = nullptr;
};



#endif // FMUVariableTableModelH
