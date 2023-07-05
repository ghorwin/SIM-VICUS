#ifndef SVACOUSTICCONSTRAINTSCHECKDIALOG_H
#define SVACOUSTICCONSTRAINTSCHECKDIALOG_H

#include <QDialog>

namespace Ui {
class SVAcousticConstraintsCheckDialog;
}

class SVAcousticConstraintsCheckDialog : public QDialog
{
    Q_OBJECT

public:
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
	void renderConstraints();
	/*! gets triggered by an entry in the tableWidget and selects and focuses the corresponding surfaces */
	void showSurfaces(unsigned int surfaceAId, unsigned int surfaceBId);
	/*! tells if a constraint was violated or not or not even existing*/
	enum ViolationInfo {
		VI_Not_Violated,
		VI_Violated,
		VI_No_Constraint,
		NUM_VI
	};

	enum wallTableColumn {
		WTC_acousticTemplateA,
		WTC_acousticTemplateB,
		WTC_acousticComponent,
		WTC_actualAirSoundValue,
		WTC_normalConstraints,
		WTC_advancedConstraints,
		WTC_sameStructure,
		WTC_showButton,

	};

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


	Ui::SVAcousticConstraintsCheckDialog *m_ui;

	/*! checks the acoustic constraints and outputs the results in the widgets table*/
	void checkConstraints();

	/*! stores all the table entries for walls*/
	std::vector<tableEntry> m_wallTes;

	/*! stores all the table entries for ceilings*/
	std::vector<tableEntry> m_ceilingTes;


	bool m_hideWalls = true;
	bool m_hideCeilings = true;

};

#endif // SVACOUSTICCONSTRAINTSCHECKDIALOG_H
