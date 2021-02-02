#ifndef SVDBNetworkComponentEditWidgetH
#define SVDBNetworkComponentEditWidgetH

#include <QWidget>

namespace VICUS {
	class NetworkComponent;
}

class SVDBNetworkComponentTableModel;
class SVDatabase;

namespace Ui {
	class SVDBNetworkComponentEditWidget;
}

/*! Edit widget for network components.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBNetworkComponentEditWidget : public QWidget {
	Q_OBJECT

public:

	explicit SVDBNetworkComponentEditWidget(QWidget *parent = nullptr);
	~SVDBNetworkComponentEditWidget();

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBNetworkComponentTableModel * dbModel);

	/*! Update widget with this. */
	void updateInput(int id);


private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxComponentType_currentIndexChanged(int index);

	void on_pushButtonComponentColor_colorChanged();

	void on_tableWidgetParameters_cellChanged(int row, int column);

private:
	Ui::SVDBNetworkComponentEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBNetworkComponentTableModel		*m_dbModel;

	/*! Pointer to currently edited component.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no component to edit.
	*/
	VICUS::NetworkComponent				*m_currentComponent;
};

#endif // SVDBNetworkComponentEditWidgetH
