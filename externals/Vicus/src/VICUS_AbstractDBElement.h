#ifndef VICUS_ABSTRACTDBELEMENT_H
#define VICUS_ABSTRACTDBELEMENT_H

namespace VICUS {

class AbstractDBElement {
public:
	enum ComparisonResult {
		/*! All parameters are identical (Caution: rounding errors!), except maybe the ID. */
		Equal,
		/*! All critical parameters are identical, yet some metadata (name etc.) may be different. */
		OnlyMetaDataDiffers,
		/*! Objects are completely different (at least one critical parameter differs). */
		Different
	};

	/*! Virtual destructor, definition is in VICUS_Material.cpp. */
	virtual ~AbstractDBElement();

	/*! Comparison of database element by content, without ID. */
	virtual ComparisonResult equal(const AbstractDBElement * other) const = 0;

	/* If true, this is a built-in DB element and cannot be modified/removed. */
	bool m_builtIn = false;
};

} // namespace VICUS

#endif // VICUS_ABSTRACTDBELEMENT_H
