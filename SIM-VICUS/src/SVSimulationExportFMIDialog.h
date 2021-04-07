#ifndef SVSimulationExportFMIDialogH
#define SVSimulationExportFMIDialogH

#include <QDialog>

#include <VICUS_Project.h>

namespace Ui {
	class SVSimulationExportFMIDialog;
}

class QTableWidgetItem;

/*! The start dialog for a NANDRAD simulation.
	Contains pages for all global simulation properties.
*/
class SVSimulationExportFMIDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationExportFMIDialog(QWidget *parent = nullptr);
	~SVSimulationExportFMIDialog();

	int edit();

	/*! Returns a copy of the locally modified version of the project. */
	const VICUS::Project & localProject() const { return m_localProject; }

private slots:
	void on_pushButtonClose_clicked();
	void on_pushButtonUpdateVariableList_clicked();

	void on_tableWidgetInputVars_itemChanged(QTableWidgetItem *item);

private:
	struct IDInfo {
		std::vector<unsigned int>	m_objectIDs;
		std::vector<unsigned int>	m_vectorIndexes;
	};

	void updateVariableLists(bool silent);

	bool parseVariableList(const QString & varsFile, std::map<QString, IDInfo> & modelVariables, bool silent);

	Ui::SVSimulationExportFMIDialog		*m_ui;

	VICUS::Project						m_localProject;

	std::map<QString, IDInfo>			m_modelInputVariables;
	std::map<QString, IDInfo>			m_modelOutputVariables;
};

#endif // SVSimulationExportFMIDialogH
