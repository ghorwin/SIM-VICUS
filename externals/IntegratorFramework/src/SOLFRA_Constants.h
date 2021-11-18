#ifndef SOLFRA_ConstantsH
#define SOLFRA_ConstantsH

namespace SOLFRA {

extern const char * const VERSION;
extern const char * const LONG_VERSION;

#define SUNDIALS_TIMER_WRITE_OUTPUTS 19
#define SUNDIALS_TIMER_STEP_COMPLETED 20

#define SOLFRA_NOT_SUPPORTED_FUNCTION (std::size_t)-1

} // namespace SOLFRA

// dump routines helper
//#define DUMP_JACOBIAN_BINARY
//#define DUMP_JACOBIAN_POSTSCRIPT
//#define DUMP_JACOBIAN_TEXT


#endif // SOLFRA_ConstantsH
