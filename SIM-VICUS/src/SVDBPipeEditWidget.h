#ifndef SVDBPIPEEDITWIDGET_H
#define SVDBPIPEEDITWIDGET_H

#include <QWidget>

namespace VICUS {
class NetworkPipe;
}

class SVDBPipeTableModel;
class SVDatabase;


namespace Ui {
class SVDBPipeEditWidget;
}

class SVDBPipeEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVDBPipeEditWidget(QWidget *parent = nullptr);
	~SVDBPipeEditWidget();

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBPipeTableModel * dbModel);

	/*! Update widget with this. */
	void updateInput(int id);


private slots:
	void on_lineEditName_editingFinished();
	void on_lineEditOuterDiameter_editingFinished();

	void on_lineEditWallThickness_editingFinished();

	void on_lineEditWallLambda_editingFinished();

	void on_lineEditWallRoughness_editingFinished();

	void on_lineEditInsulationThickness_editingFinished();

	void on_lineEditInsulationLambda_editingFinished();

private:
	Ui::SVDBPipeEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBPipeTableModel		*m_dbModel;

	/*! Pointer to currently edited pipe.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no material to edit.
	*/
	VICUS::NetworkPipe			*m_current;
};


#endif // SVDBPIPEEDITWIDGET_H
