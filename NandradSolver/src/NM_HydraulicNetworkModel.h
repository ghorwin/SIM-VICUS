#ifndef NM_HydraulicNetworkModelH
#define NM_HydraulicNetworkModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class HydraulicNetwork;
}


namespace NANDRAD_MODEL {

class HydraulicNetworkModelImpl;

/*! A model for a hydraulic network.

	Network model computes pressure and mass flux distribution in the network.
	Interaction with thermal zones (i.e. load calculation and thermal transport)
	is implemented in a different model.

	The hydraulic network depends at runtime from other model inputs, for example through controlled valve
	settings (may be supplied via scheduled parameters) and thermal properties (that may impact kinematic fluid viscosity).
*/
class HydraulicNetworkModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Constructor. */
	HydraulicNetworkModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! D'tor, released pimpl object. */
	~HydraulicNetworkModel() override;

	/*! Initializes model.
		\param nw The hydraulic network model definition/parametrization.
	*/
	void setup(const NANDRAD::HydraulicNetwork & nw);


	// *** Re-implemented from AbstractModel

	/*! Balance model can be referenced as ConstructionInstance and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_NETWORK;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "HydraulicNetworkModel";}

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Returns display name of this model instance. */
	virtual const char * displayName() const override { return m_displayName.c_str(); }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\note For quantity 'ydot' the memory with computed ydot-values is returned.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Composes all input references. */
	virtual void initInputReferences(const std::vector<AbstractModel*> & models) override;

	/*! Returns vector with model input references. */
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Provides the object with references to requested input variables (persistent memory location). */
	virtual void setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) override;

	/*! Returns dependencies between result variables and input variables. */
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


private:

	/*! Construction instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Private implementation (Pimpl) of the network solver. */
	HydraulicNetworkModelImpl						*m_p = nullptr;

};

} // namespace NANDRAD_MODEL

#endif // NM_HydraulicNetworkModelH
