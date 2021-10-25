#ifndef SOLFRA_ConstantsH
#define SOLFRA_ConstantsH

#include <cstddef>

namespace SOLFRA {

extern const char * const VERSION;
extern const char * const LONG_VERSION;

#define SUNDIALS_TIMER_WRITE_OUTPUTS 19
#define SUNDIALS_TIMER_STEP_COMPLETED 20

/*! Defines return value of non supported serialization functionality */
extern std::size_t NON_SUPPORTED_FUNCTION;

} // namespace SOLFRA

// dump routines helper
//#define DUMP_JACOBIAN_BINARY
//#define DUMP_JACOBIAN_POSTSCRIPT
//#define DUMP_JACOBIAN_TEXT


#endif // SOLFRA_ConstantsH
