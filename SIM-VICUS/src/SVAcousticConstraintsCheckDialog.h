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
	enum ColumnsSoundProtection {
		CSP_AcousticTemplateA,
		CSP_AcousticTemplateB,
		CSP_AcousticComponent,
		CSP_ActualAirSoundValue,
		CSP_NormalConstraints,
		CSP_AdvancedConstraints,
		CSP_SameStructure,
		CSP_SelectButton,
		NUM_CSP
	};

	/*! Enum for columns. */
	enum ColumnsReverberationTime {
		CRT_RoomID,
		CRT_RoomName,
		CRT_Reverb125Hz,
		CRT_Reverb250Hz,
		CRT_Reverb500Hz,
		CRT_Reverb1000Hz,
		CRT_Reverb2000Hz,
		CRT_Reverb4000Hz,
		NUM_CRT
	};

	/*! Struct for table entries. */
	struct tableEntry{
		bool			m_isImpact;
		bool			m_isSameStructuralUnit;
		QString			m_acousticTemplateAInfo;
		QString			m_acousticTemplateBInfo;
		QString			m_acousticComponentInfo;
		ViolationInfo	m_basicConstraintViolated;
		QString			m_actualValue;
		QString			m_expectedNormalLimit;
		ViolationInfo	m_advancedConstraintViolated;
		QString			m_expectedAdvancedLimit;
		// those are not directely displayed, but needed to select the surfaces on the show button
		unsigned int	m_surfaceAId;
		unsigned int	m_surfaceBId;
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
