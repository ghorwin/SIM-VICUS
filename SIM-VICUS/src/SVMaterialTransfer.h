#ifndef SVMaterialTransferH
#define SVMaterialTransferH

#include <QtExt_MaterialBase.h>
#include <VICUS_Material.h>

class SVMaterialTransfer : public QtExt::MaterialBase
{
public:
	SVMaterialTransfer(const VICUS::Material& mat);

	/*! The name of the material (already internationalized).*/
	virtual QString						name() const override;

	/*! The material category (used for grouping the material).*/
	virtual QtExt::MaterialCategory		category() const override;

	/*! Default color used for showing the material.*/
	virtual QColor						color() const override { return m_mat.m_color; }

	/*! Information and notes for the product.*/
	virtual QString						notes() const override { return m_mat.m_notes; }

	/*! Product-ID or brand name.*/
	virtual QString						productName() const override { return QString(); }

	/*! Name of producer or provider.*/
	virtual QString						producer() const override { return m_mat.m_manufacturer; }

	/*! Name of data source.*/
	virtual QString						dataSource() const override { return m_mat.m_dataSource; }

	/*! Bulk density in [kg/m3].*/
	virtual double						rho() const override { return m_mat.m_para[VICUS::Material::P_Density].get_value("kg/m3"); }

	/*! Specific heat capacity in [Ws/kgK].*/
	virtual double						cp() const override { return m_mat.m_para[VICUS::Material::P_Density].get_value("Ws/kgK"); }

	/*! Vapor diffusion resistance factor (optional, minimum value) [-].*/
	virtual double						mu() const override { return m_mat.m_para[VICUS::Material::P_Density].get_value("-"); }

	/*! Thermal conductivity in [W/mK].*/
	virtual double						lambda() const override { return m_mat.m_para[VICUS::Material::P_Density].get_value("W/mK]"); }

	/*! Hygroscopic moisture content at 80% RH in [kg/m3].*/
	virtual double						w_80() const override { return m_mat.m_para[VICUS::Material::P_Density].get_value("kg/m3"); }

	/*! Saturation moisture content in [kg/m3].*/
	virtual double						w_sat() const override { return m_mat.m_para[VICUS::Material::P_Density].get_value("kg/m3"); }

	/*! Returns the material id.*/
	virtual int materialId() const override {return static_cast<int>(m_mat.m_id); }

	/*! Return if a material is a user material able to edit.*/
	virtual bool isUserMaterial() const override {return false; }

	/*! Return combination of transport capabilities.*/
	virtual int capabilities() const override {return QtExt::MaterialBase::TT_Thermal; }

	/*! Return if a material is marked as deprecated (value > 0) or deleted (value = -1).*/
	int deprecatedState() const { return 0; }

	/*! Returns a string of the parameter value. Formating depends on value type.*/
	QString materialValueFormatter(parameter_t type, double value) {
		if(m_valueFormatter)
			return m_valueFormatter(int(type),value);
		return defaultMaterialValueFormatter(type,value);
	}
private:
	/*! Vicus material. */
	VICUS::Material					m_mat;
};

#endif // SVMaterialTransferH
