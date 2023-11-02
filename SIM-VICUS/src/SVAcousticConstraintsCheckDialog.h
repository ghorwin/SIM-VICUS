#ifndef SVACOUSTICCONSTRAINTSCHECKDIALOG_H
#define SVACOUSTICCONSTRAINTSCHECKDIALOG_H

#include <QDialog>

namespace Ui {
class SVAcousticConstraintsCheckDialog;
}


/*! Dialog for checking acoustic constrains dynamically. */
class SVAcousticConstraintsCheckDialog : public QDialog {
	Q_OBJECT

public:

	/*! Violation information. */
	enum ViolationInfo {
		VI_Valid,
		VI_Invalid,
		VI_NoConstraint,
		NUM_VI
	};

	/*! Enum for columns. */
	enum Columns {
		ColAcousticTemplateA,
		ColAcousticTemplateB,
		ColAcousticComponent,
		ColActualAirSoundValue,
		ColNormalConstraints,
		ColAdvancedConstraints,
		ColSameStructure,
		ColSelectButton,
	};

	/*! Struct for table entries. */
	struct tableEntry{
		bool isImpact;
		bool isSameStructuralUnit;
		QString acousticTemplateAInfo;
		QString acousticTemplateBInfo;
		QString acousticComponentInfo;
		ViolationInfo basicConstraintViolated;
		QString actualValue;
		QString expectedNormalLimit;
		ViolationInfo advancedConstraintViolated;
		QString expectedAdvancedLimit;
		// those are not directely displayed, but needed to select the surfaces on the show button
		unsigned int surfaceAId;
		unsigned int surfaceBId;
	};


	explicit SVAcousticConstraintsCheckDialog(QWidget *parent = nullptr);
	~SVAcousticConstraintsCheckDialog();

	/*! Always start the dialog with this function.
		\return Returns true if dialog was confirmed and data can be added to project.
	*/
	bool edit();

private slots:
	void on_pushButtonCheckConstraints_clicked();

	void on_checkBoxHideWalls_stateChanged(int arg1);

	void on_checkBoxHideCeilings_stateChanged(int arg1);

private:
	/*! fills the table widgets with the stored entries*/
	void updateTable();

	/*! gets triggered by an entry in the tableWidget and selects and focuses the corresponding surfaces */
	void showSurfaces(unsigned int surfaceAId, unsigned int surfaceBId);
	/*! tells if a constraint was violated or not or not even existing*/
	/*! checks the acoustic constraints and outputs the results in the widgets table*/
	void checkConstraints();


	void checkReverberation();

	/*! Pointer to Ui. */
	Ui::SVAcousticConstraintsCheckDialog *m_ui;

	/*! stores all the table entries for walls*/
	std::vector<tableEntry> m_wallTes;

	/*! stores all the table entries for ceilings*/
	std::vector<tableEntry> m_ceilingTes;

	/*! Indicates wheather walls should be hidden. */
	bool m_hideWalls = true;

	/*! Indicates wheather walls should be hidden. */
	bool m_hideCeilings = true;

};

#endif // SVACOUSTICCONSTRAINTSCHECKDIALOG_H
