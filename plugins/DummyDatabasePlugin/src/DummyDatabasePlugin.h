#ifndef DummyDatabasePluginH
#define DummyDatabasePluginH

#include <QObject>
#include <QtPlugin>

#include <SVDatabasePluginInterface.h>

class DummyDatabasePlugin : public QObject, public SVDatabasePluginInterface {
public:
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "ibk.sim-vicus.Plugin.DatabaseInterface")  // optionally add  'FILE "DatabaseProperties.json"'
	Q_INTERFACES(SVDatabasePluginInterface)
public:

	// SVCommonPluginInterface interface
	QString title() const override { return "Database-Plugin-Dummy"; }
	bool hasSettingsDialog() const override { return true;}
	int showSettingsDialog(QWidget * parent) override;

	// SVDatabasePluginInterface interface
	bool retrieve(const SVDatabase & currentDB, SVDatabase & augmentedDB) override;
};

#endif // DummyDatabasePluginH

