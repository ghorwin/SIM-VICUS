#include "FMUVariableTableModel.h"

#include "NandradFMUGeneratorWidget.h"

#include <QColor>

FMUVariableTableModel::FMUVariableTableModel(QObject * parent, bool inputVariableTable) :
	QAbstractTableModel(parent),
	m_inputVariableTable(inputVariableTable)
{

}

int FMUVariableTableModel::rowCount(const QModelIndex & /*parent*/) const {
	return m_availableVariables->size();
}


int FMUVariableTableModel::columnCount(const QModelIndex & /*parent*/) const {
	return 8;
}


QVariant FMUVariableTableModel::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();
	const NANDRAD::FMIVariableDefinition & var = (*m_availableVariables)[(size_t)index.row()];
	switch (role) {
		case Qt::DisplayRole :
			switch (index.column()) {
				case 0 : // name
					return QString::fromStdString(var.m_varName);
				case 1 : // object ID
					return var.m_objectId;
				case 2 : // vector value ID
					if (var.m_vectorIndex == NANDRAD::INVALID_ID)
						return QVariant();
					return var.m_vectorIndex;
				case 3 : // unit
					return QString::fromStdString(var.m_unit);
				case 4 : // FMI Variable Name
					return QString::fromStdString(var.m_fmiVarName);
				case 5 : // FMI value ref
					if (var.m_fmiValueRef == NANDRAD::INVALID_ID)
						return "---";
					return var.m_fmiValueRef;
				case 6 : // FMI type
					return QString::fromStdString(var.m_fmiTypeName);
				case 7 : // Description
					return QString::fromStdString(var.m_fmiVarDescription);
			}
		break;

		case Qt::EditRole :
			Q_ASSERT(index.column() == 4);
			return QString::fromStdString(var.m_fmiVarName);

		case Qt::FontRole :
			// vars with INVALID valueRef -> grey italic
			//      with valid value -> black, bold
			if (var.m_fmiValueRef == NANDRAD::INVALID_ID) {
				QFont f(m_itemFont);
				f.setItalic(true);
				return f;
			}
			else {
				QFont f(m_itemFont);
				f.setBold(true);
				return f;
			}

		case Qt::ForegroundRole :
			// vars with INVALID valueRef -> grey italic
			if (var.m_fmiValueRef == NANDRAD::INVALID_ID)
				return QColor(Qt::gray);
		break;

		// UserRole returns value reference
		case Qt::UserRole :
			return var.m_fmiValueRef;
	}
	return QVariant();
}


QVariant FMUVariableTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	static QStringList headers = QStringList()
		<< tr("NANDRAD Variable Name")
		<< tr("Object ID")
		<< tr("Vector value index/ID")
		<< tr("Unit")
		<< tr("FMI Variable Name")
		<< tr("FMI value reference")
		<< tr("FMI Type")
		<< tr("Description");

	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole :
			return headers[section];
	}
	return QVariant();
}


Qt::ItemFlags FMUVariableTableModel::flags(const QModelIndex & index) const {
	if (index.column() == 4)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	return QAbstractTableModel::flags(index); // call base class implementation
}


bool FMUVariableTableModel::setData(const QModelIndex & index, const QVariant & value, int /*role*/) {
	Q_ASSERT(index.isValid());
	Q_ASSERT(index.column() == 4);
	// error handling
	QString fmiVarName = value.toString().trimmed();
	// variable name must not be empty or only consist of white spaces; this is silently handled as error (because obvious)
	if (fmiVarName.isEmpty())
		return false;

	// Note: NANDRAD FMUs use structured variable naming, to "." is ok as variable name, like in "Office.AirTemperature"
	//
	// See FMI Standard 2.2.7: "name: The full, unique name of the variable. Every variable is uniquely identified within an
	//                          FMU instance by this name"
	//
	// Every FMU variable shall have a unique name. So we must test if the newly entered FMU name is anywhere used already.
	// However, we have different handling for output variables and input variables.
	//
	// Output variables:
	// (1) user must not select a variable name that is already used for another output/input variable; in such cases
	//     the function shall return false and the data shall not be modified.
	//
	// Input variables:
	// (2) user must not select a variable name that is already used for another *output* variable; in such cases
	//     the function shall return show an error message and return false and the data shall not be modified.
	//
	// (3) if user has edited a variable with unique name and has now selected a name that is already used by another
	//     *input* variable, we expect:
	//   - the value reference assigned to the other, existing variable shall be set also to the freshly renamed variable,
	//     this both NANDRAD model variables share the same (single and unique) FMU input variable
	//   - if the existing other variable with same name has a different unit, the renaming is invalid, an error message
	//     shall be shown and the function returns with false (no data is modified)
	//
	//     Rational: in more complex models, an externally computed control parameter may be needed as input for many
	//               NANDRAD model objects. In such situations the FMU may only have one external FMI input variable
	//               that is, however, configured to be linked to several NANDRAD model object inputs. Hence, it must
	//               be possible to assign the same variable name and value reference to different NANDRAD variables in
	//               the table.
	//
	// (4) if user has edited a variable that shares the same name and value reference as another input variable,
	//     first the condition for (3) shall be checked and if matching, the steps for option (3) shall be followed.
	//     If the new variable name is unique, the FMI variable shall receive a new unique value reference.

	// get variable that we modified
	NANDRAD::FMIVariableDefinition & var = (*m_availableVariables)[(size_t)index.row()];

	// ask Widged to rename and check variable
	NandradFMUGeneratorWidget *parentWidged = dynamic_cast<NandradFMUGeneratorWidget *>(parent());
	Q_ASSERT(parentWidged != nullptr);

	if(m_inputVariableTable) {
		// error handling is performed in 'NandradFMUGeneratorWidget'
		if(!parentWidged->renameInputVariable(var, fmiVarName) )
			return false;
	}
	else {
		// error handling is performed in 'NandradFMUGeneratorWidget'
		if(!parentWidged->renameOutputVariable(var, fmiVarName) )
			return false;
	}

	emit dataChanged(index, index);
	return true;
}


void FMUVariableTableModel::reset() {
	beginResetModel();
	endResetModel();
}


void FMUVariableTableModel::variableModified(unsigned int row) {
	// get index range
	QModelIndex left = index((int)row, 0);
	QModelIndex right = index((int)row, 7);
	emit dataChanged(left, right);
}
