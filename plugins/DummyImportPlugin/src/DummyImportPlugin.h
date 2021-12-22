#ifndef DummyImportPluginH
#define DummyImportPluginH

#include <QObject>
#include <QtPlugin>

#include <SVImportPluginInterface.h>

class DummyImportPlugin : public QObject, public SVImportPluginInterface {
public:
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "ibk.sim-vicus.Plugin.ImportInterface")  // optionally add  'FILE "ImporterProperties.json"'
	Q_INTERFACES(SVImportPluginInterface)

public:

	// SVCommonPluginInterface interface

	bool haveSettingsDialog() const override { return true;}
	int showSettingsDialog(QWidget * parent) override;

	// SVImportPluginInterface interface

	QString importMenuCaption() const override;
	bool import(QWidget * parent, VICUS::Project & p) override;
};

#endif // DummyImportPluginH

