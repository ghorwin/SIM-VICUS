#ifndef QtExt_ConstructionViewH
#define QtExt_ConstructionViewH

#include <QObject>
#include <QGraphicsView>
#include <QVector>
#include <QPair>
#include <QString>
#include <QPrinter>
#include <QResizeEvent>

#include "QtExt_ConstructionGraphicsScene.h"
#include "QtExt_ConstructionLayer.h"

namespace QtExt {

class ConstructionView : public QGraphicsView
{
	Q_OBJECT
public:
	static const QColor ColorList[12];

	/*! Default constructor.
		\param parent Parent widget.
	*/
	explicit ConstructionView(QWidget *parent = 0);

	~ConstructionView();

	/*! Updates the locally cached data.*/
	void setData(QPaintDevice* paintDevice, const QVector<ConstructionLayer>& layers, double resolution);

	/*! Clears content and scene.*/
	void clear();

	/*! Creates a svg image and paint it to the outputDevice.*/
	void createSvg(QIODevice * outputDevice);

	/*! Creates a pixmap.*/
	QPixmap createPixmap();

	/*! Write to the given printer.*/
	void print(QPrinter* printer);

	/*! Deselect all selected items.*/
	void deselectItems();

	/*! Return selected layer.*/
	int selectedLayer() const { return m_selectedLayer; }

signals:
	/*! Signal that contains selected layer or -1 if no layer is selected.*/
	void layerSelected(int index);
	/*! Signal that contains selected layer or -1 if no layer is selected in case of double click.*/
	void layerDoubleClicked(int index);

public slots:

	void selectLayer(int index);

protected:
	/*! Is called if a resize is necessary.
		Set new size for the diagramScene object.
	*/
	virtual void resizeEvent ( QResizeEvent * event);

	virtual void mousePressEvent(QMouseEvent *event);

	virtual void mouseDoubleClickEvent(QMouseEvent *event);

	/*! New paint event can draw items independently from scene.*/
	void paintEvent ( QPaintEvent * event );

private slots:
	/*! Connected to graphicsscene selectionChanged signal.*/
	void sceneSelectionChanged();
	/*! Connected to graphicsscene doubleClick signal.*/
	void sceneDoubleClicked();

private:
	QPaintDevice*		m_device;					///< Paintdevice.

	/*! Local copy of all input data to be used by the drawing code.
		We store a local copy of the data so that graphics updates can be
		done individually from outer code.
	*/
	QVector<ConstructionLayer> m_inputData;

	/*! Paints the diagram.*/
	ConstructionGraphicsScene*	m_diagramScene;

	/*! Margins between view and scene.*/
	int				m_margins;

	/*! Resolution of page (pixel per millimeter).*/
	double			m_resolution;

	/*! Currently selected layer. Is -1 if nothing is selected or no layer exist.*/
	int				m_selectedLayer;
};

} // namespace QtExt

#endif // QTEXT_CONSTRUCTIONVIEW_H
