#ifndef SVPropEditGeometryH
#define SVPropEditGeometryH

#include <QWidget>

namespace Ui {
class SVPropEditGeometry;
}

/*! This widget is shown when the scene is put into geometry editing mode. */
class SVPropEditGeometry : public QWidget {
	Q_OBJECT


public:
	enum TabState {
		TS_AddGeometry,
		TS_EditGeometry,
		NUM_TS
	};

	explicit SVPropEditGeometry(QWidget *parent = nullptr);
	~SVPropEditGeometry();

	/*! Sets the current tab index to the TabState specified
	*/
	void setCurrentTab(const TabState &state);

private slots:
	void on_pushButtonAddPolygon_clicked();

	void on_toolButtonAddZoneBox_clicked();

private:
	Ui::SVPropEditGeometry *m_ui;
};

#endif // SVPropEditGeometryH
