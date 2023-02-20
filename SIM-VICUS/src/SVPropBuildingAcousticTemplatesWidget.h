#ifndef SVPROPBUILDINGACOUSTICTEMPLATESWIDGET_H
#define SVPROPBUILDINGACOUSTICTEMPLATESWIDGET_H

#include <QWidget>

namespace Ui {
class SVPropBuildingAcousticTemplatesWidget;
}

namespace VICUS {
	class AcousticTemplate;
	class Room;
}

class ModificationInfo;

class SVPropBuildingAcousticTemplatesWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropBuildingAcousticTemplatesWidget(QWidget *parent = nullptr);
	~SVPropBuildingAcousticTemplatesWidget();

/*! Updates user interface. */
void updateUi();

private slots:
	void on_tableWidgetAcousticTemplates_itemSelectionChanged();

	void on_pushButtonAssignAcousticTemplate_clicked();

	void onModified(int modificationType, ModificationInfo * /*data*/);

	void on_pushButtonDeleteTemplate_clicked();

private:

	/*! Returns a pointer to the currently selected acoustic template in the zone template table. */
	const VICUS::AcousticTemplate * currentlySelectedAcousticTemplate() const;


	Ui::SVPropBuildingAcousticTemplatesWidget *m_ui;

	/*! Maps stores pointers to room objects grouped for assigned acoustic templates.
		Note: rooms without zone template ID are ignored.
	*/
	std::map<const VICUS::AcousticTemplate*, std::vector<const VICUS::Room *> >			m_acousticTemplateAssignments;
};

#endif // SVPROPBUILDINGACOUSTICTEMPLATESWIDGET_H
