#include "QtExt_ConstructionView.h"

#include <QSvgGenerator>
#include <QDebug>

namespace QtExt {

const QColor ConstructionView::ColorList[12] = {
	   QColor(0, 255, 255),
	   QColor(222, 184, 135),
	   QColor(127, 255, 0),
	   QColor(255, 127, 80),
	   QColor(100, 149, 237),
	   QColor(220, 20, 60),
	   QColor(189, 183, 107),
	   QColor(255, 140, 0),
	   QColor(143, 188, 143),
	   QColor(255, 20, 147),
	   QColor(30, 144, 255),
	   QColor(255, 215, 0),
   };

ConstructionView::ConstructionView(QWidget *parent) :
	QGraphicsView(parent),
	m_device(0),
	m_diagramScene(new ConstructionGraphicsScene(true, this)),
	m_margins(5),
	m_resolution(1.0),
	m_selectedLayer(-1)
{
	QSizePolicy p(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	p.setHorizontalStretch(1);
	p.setVerticalStretch(1);
	setSizePolicy(p);
	setInteractive(false);
	setScene(m_diagramScene);
	connect(m_diagramScene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));

	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	setCacheMode(QGraphicsView::CacheBackground);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	setOptimizationFlags(QGraphicsView::DontClipPainter
						 | QGraphicsView::DontSavePainterState
						 | QGraphicsView::DontAdjustForAntialiasing);
	setAlignment(Qt::AlignLeft | Qt::AlignBottom);


}

ConstructionView::~ConstructionView() {
	delete m_diagramScene;
}

void ConstructionView::setData(QPaintDevice* paintDevice, const QVector<ConstructionLayer>& layers, double resolution) {
	m_inputData = layers;
	m_resolution = resolution;
	m_device = paintDevice;
	Q_ASSERT(m_device);
	int w = width();
	int h = height();
	QRect frame(m_margins, m_margins, w - m_margins * 2, h - m_margins * 2);
	m_diagramScene->setup(frame, m_device, m_resolution, m_inputData);
	show();
}

void ConstructionView::clear() {
	m_inputData.clear();
	m_diagramScene->clear();
}

void ConstructionView::resizeEvent( QResizeEvent * event) {
	QSize diffSize = event->size() - event->oldSize();
	if( !m_device)
		return;

	if( std::abs(diffSize.width()) > m_margins - 1 ||  std::abs(diffSize.height()) > m_margins - 1) {
		QRect frame(QPoint(0,0), event->size());
		frame = frame.adjusted(m_margins, m_margins, m_margins * -1, m_margins * -1);
		m_diagramScene->setup(frame, m_device, m_resolution, m_inputData);
	}
}

void ConstructionView::paintEvent ( QPaintEvent * event ) {
	QGraphicsView::paintEvent(event);
}

void ConstructionView::print(QPrinter* printer) {

	printer->setPaperSize(QPrinter::A4);
	double widgetWidth = width();
	double widgetHeight = height();
	int paperWidth = printer->pageRect().width();
	double scale = paperWidth / widgetWidth;
	int w = widgetWidth * scale;
	int h = widgetHeight * scale;
	QRect frame(0, 0, w, h);

	QPainter painter;
	painter.begin(printer);
	ConstructionGraphicsScene sketch(true, printer);
	sketch.setup(frame, printer, 1.0, m_inputData);

	sketch.render(&painter);
	painter.end();
}

QPixmap ConstructionView::createPixmap() {
	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);
	int paperWidth = printer.pageRect().width();
	double scale = paperWidth / double(width());
	int w = width() * scale;
	int h = height() * scale;
	QPixmap pixmap(w, h);
	pixmap.fill(Qt::white);
	QRect frame(0, 0, w, h);
	QPainter painter;
	painter.begin(&pixmap);

	ConstructionGraphicsScene sketch(true, &pixmap);
	sketch.setup(frame, &pixmap, 1.0, m_inputData);

	sketch.render(&painter);

	painter.end();
	return pixmap;
}

void ConstructionView::createSvg(QIODevice * outputDevice) {
	QSvgGenerator generator;
	generator.setOutputDevice(outputDevice);
	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);
	int paperWidth = printer.pageRect().width();
	double scale = paperWidth / double(width());
	int w = width() * scale;
	int h = height() * scale;
	QRect frame(0, 0, w, h);
	generator.setSize(frame.size());
	generator.setViewBox(frame);
	generator.setTitle(tr("Construction Sketch"));

	generator.setTitle(tr("Construction Sketch"));
	generator.setDescription("");
	QPainter painter;
	painter.begin(&generator);
	ConstructionGraphicsScene sketch(true, &generator);
	sketch.setup(frame, &generator, 1.0, m_inputData);

	sketch.render(&painter);
	painter.end();
}

void ConstructionView::mousePressEvent(QMouseEvent *event) {
	if(event->button() == Qt::LeftButton) {
		QPointF pos = event->localPos();
		QGraphicsItem* item = itemAt(pos.toPoint());
		QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(item);
		if(rectItem != 0) {
			bool selected = rectItem->isSelected();
			if(!selected) {
				deselectItems();
				rectItem->setZValue(1);
				rectItem->setSelected(true);
			}
		}
	}

	QGraphicsView::mousePressEvent(event);

}

void ConstructionView::mouseDoubleClickEvent(QMouseEvent *event) {
	QPointF pos = event->localPos();
	QGraphicsItem* item = itemAt(pos.toPoint());
	QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(item);
	if(rectItem != 0) {
		bool selected = rectItem->isSelected();
		if(!selected) {
			deselectItems();
			rectItem->setZValue(1);
			rectItem->setSelected(true);
		}
		emit layerDoubleClicked(m_selectedLayer);
	}
	QGraphicsView::mouseDoubleClickEvent(event);
}

void ConstructionView::deselectItems() {
	QList<QGraphicsItem *> selected = m_diagramScene->selectedItems();
	if(!selected.empty()) {
		for(int i=0; i<selected.size(); ++i) {
			QGraphicsRectItem* rect = dynamic_cast<QGraphicsRectItem*>(selected[i]);
			if(rect != nullptr) {
				rect->setZValue(0);
				rect->setSelected(false);
			}
		}
	}
}

void ConstructionView::selectLayer(int index) {
	deselectItems();
	QList<QGraphicsItem *> items = m_diagramScene->items();
	for(int i=0; i<items.size(); ++i) {
		QGraphicsItem * item = items[i];
		if(item->type() == QGraphicsRectItem::Type) {
			int rectIndex = item->data(0).toInt();
			if(index == rectIndex) {
				item->setZValue(1);
				item->setSelected(true);
				return;
			}
		}
	}
}

void ConstructionView::sceneSelectionChanged() {
	QList<QGraphicsItem *> items = m_diagramScene->selectedItems();
	int index = -1;
	if(items.size() == 1)
		index = items.front()->data(0).toInt();
	m_selectedLayer = index;
	emit layerSelected(m_selectedLayer);
}

void ConstructionView::sceneDoubleClicked() {
	QList<QGraphicsItem *> items = m_diagramScene->selectedItems();
	int index = -1;
	if(items.size() == 1)
		index = items.front()->data(0).toInt();
	m_selectedLayer = index;
	emit layerDoubleClicked(m_selectedLayer);
}


} // namespace QtExt
