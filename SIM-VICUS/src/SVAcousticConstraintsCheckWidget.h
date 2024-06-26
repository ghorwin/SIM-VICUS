#ifndef SVAcousticConstraintsCheckWidgetH
#define SVAcousticConstraintsCheckWidgetH

#include <QDialog>

namespace Ui {
class SVAcousticConstraintsCheckWidget;
}


/*! Dialog for checking acoustic constrains dynamically. */
class SVAcousticConstraintsCheckWidget : public QWidget {
	Q_OBJECT

public:

	/*! Enum for tables. */
	enum TableType {
		TT_ImpactSound,
		TT_AirBourneSound,
		NUM_TT
	};

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
		CRT_Template,
		CRT_TGoal,
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


	explicit SVAcousticConstraintsCheckWidget(QWidget *parent = nullptr);
	~SVAcousticConstraintsCheckWidget();

private slots:
	void on_pushButtonCheckConstraints_clicked();

	void on_checkBoxHideAirBourneSound_stateChanged(int hideAirBourneSound);

	void on_checkBoxHideImpactSound_stateChanged(int hideImpactSound);

private:
	/*! fills the table widgets with the stored entries*/
	void updateTable();

	/*! gets triggered by an entry in the tableWidget and selects and focuses the corresponding surfaces */
	void showSurfaces(unsigned int surfaceAId, unsigned int surfaceBId);

	/*! tells if a constraint was violated or not or not even existing
		checks the acoustic constraints and outputs the results in the widgets table. */
	void checkConstraints();

	/*! .*/
	void checkReverberation();

	/*! Pointer to Ui. */
	Ui::SVAcousticConstraintsCheckWidget *m_ui;

	/*! stores all the table entries for walls*/
	std::vector<tableEntry>				m_airBourneSoundTableEntries;

	/*! stores all the table entries for ceilings*/
	std::vector<tableEntry>				m_impactSoundTableEntries;

	/*! Indicates wheather walls should be hidden. */
	bool								m_hideAirBourneSound = true;

	/*! Indicates wheather walls should be hidden. */
	bool								m_hideImpactSound = true;

};

#endif // SVAcousticConstraintsCheckWidgetH
