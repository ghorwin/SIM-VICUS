/*subfile:  types.h  *********************************************************/

/*  redefinition of C variable types and path parameters.  */

#ifndef STD_TYPES
# define STD_TYPES
typedef char I1;           /* 1 byte signed integer */
typedef short I2;          /* 2 byte signed integer */
typedef long I4;           /* 4 byte signed integer */
typedef int IX;            /* default signed integer */
typedef unsigned char U1;  /* 1 byte unsigned integer */
typedef unsigned short U2; /* 2 byte unsigned integer */
typedef unsigned long U4;  /* 4 byte unsigned integer */
typedef unsigned UX;       /* default unsigned integer */
typedef float R4;          /* 4 byte real value */
typedef double R8;         /* 8 byte real value */
typedef long double RX;    /* 10 byte real value (extended precision) */

#define LINELEN 256
#define NAMELEN 16

#if( __GNUC__ )
#include <limits.h>  /* define PATH_MAX */
#else
#include <stdlib.h>  /* define _MAX_PATH, etc. */
#endif
#ifndef _MAX_PATH
# ifdef PATH_MAX     /* GNUC parameter defined in <limits.h> */
#  define _MAX_PATH  PATH_MAX
#  define _MAX_DIR   PATH_MAX
#  define _MAX_FNAME NAME_MAX
# else
/* _MAX_PATH, _MAX_DIR, _MAX_FNAME retain VisualC++ values */
# endif
#define _MAX_DRIVE 4    /* 3 minimum (3 VisualC++) */
#define _MAX_EXT   8    /* 5 minimum (256 VisualC++) */
/* replace _MAX_DRIVE, _MAX_EXT VisualC++ values */
#endif

#endif
