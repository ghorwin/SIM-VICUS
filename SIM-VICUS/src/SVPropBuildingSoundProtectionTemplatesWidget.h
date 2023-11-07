
#ifndef SVPropBuildingSoundProtectionTemplatesWidgetH
#define SVPropBuildingSoundProtectionTemplatesWidgetH

#include <QWidget>

namespace Ui {
class SVPropBuildingSoundProtectionTemplatesWidget;
}

namespace VICUS {
class AcousticSoundProtectionTemplate;
	class Room;
}

class ModificationInfo;

class SVPropBuildingSoundProtectionTemplatesWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropBuildingSoundProtectionTemplatesWidget(QWidget *parent = nullptr);
	~SVPropBuildingSoundProtectionTemplatesWidget();

	/*! Updates user interface. */
	void updateUi();

private slots:
	void on_pushButtonAssignAcousticTemplate_clicked();
	void on_tableWidgetAcousticTemplates_itemSelectionChanged();
	void on_pushButtonDeleteTemplate_clicked();
	void on_comboBoxBuildingType_currentIndexChanged(int);


	void onModified(int modificationType, ModificationInfo * /*data*/);

private:

	/*! Returns a pointer to the currently selected acoustic template in the zone template table. */
        const VICUS::AcousticSoundProtectionTemplate * currentlySelectedSoundProtectionTemplate() const;


	Ui::SVPropBuildingSoundProtectionTemplatesWidget *m_ui;

	/*! Maps stores pointers to room objects grouped for assigned acoustic templates.
		Note: rooms without zone template ID are ignored.
	*/
        std::map<const VICUS::AcousticSoundProtectionTemplate *, std::vector<const VICUS::Room *> >		m_acousticTemplateAssignments;
};

#endif // SVPropBuildingSoundProtectionTemplatesWidgetH
