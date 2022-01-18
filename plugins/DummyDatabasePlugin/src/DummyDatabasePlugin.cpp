#include "DummyDatabasePlugin.h"

#include <QMessageBox>

#include <VICUS_Project.h>

#include <SVDatabase.h>

int DummyDatabasePlugin::showSettingsDialog(QWidget * parent) {
	// spawn settings dialog
	QMessageBox::information(parent, QString(), tr("Here is the settings dialog for the plugin."));
	return NoUpdate;
}


bool DummyDatabasePlugin::retrieve(const SVDatabase & /*currentDB*/, SVDatabase & augmentedDB) {
	// just add a dummy material, construction and component
	VICUS::Material m;
	m.m_displayName.setEncodedString("de:Dummy-Material|en:Dummy material");
	m.m_dataSource.setEncodedString("de:Dummy Database Plugin|en:Dummy Database Plugin");
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Mu, 81);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Density, 2150);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_HeatCapacity, 1000);
	VICUS::KeywordList::setParameter(m.m_para, "Material::para_t", VICUS::Material::P_Conductivity, 2.7);

	// generate some unique ID
	m.m_id = 5500000; // our ID space

	// check, that we don't conflict with existing materials
	if (augmentedDB.m_materials[m.m_id] != nullptr) {
		IBK::IBK_Message("Duplicate ID space detected, cannot import material.", IBK::MSG_ERROR);
		return false;
	}

	unsigned int matID = augmentedDB.m_materials.add(m, m.m_id);
	augmentedDB.m_materials[matID]->m_builtIn = true;
	// TODO : later we may need something to identify this material as "plugin-based" and
	//        avoid storing it in projects
	IBK::IBK_Message(IBK::FormatString("    %1 materials added\n").arg(1), IBK::MSG_PROGRESS);
	return true;
}
