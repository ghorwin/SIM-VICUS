#ifndef ticppIBKconfigH
#define ticppIBKconfigH

// In our custom extensions we use STL and TICPP, so we just define the flag fixed here
// If the library is to be used in a C environment, we need to remove this again.
#define TIXML_USE_TICPP
#define TIXML_USE_STL
// we also wrap all our extensions in an ifdef clause, which we define below
#define TIXML_USE_IBK_EXTENSIONS

#endif // ticppIBKconfigH
