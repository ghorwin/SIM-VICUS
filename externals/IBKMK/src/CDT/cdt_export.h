
#ifndef CDT_EXPORT_H
#define CDT_EXPORT_H

#ifdef CDT_STATIC_DEFINE
#  define CDT_EXPORT
#  define CDT_NO_EXPORT
#else
#  ifndef CDT_EXPORT
#    ifdef CDT_EXPORTS
        /* We are building this library */
#      define CDT_EXPORT 
#    else
        /* We are using this library */
#      define CDT_EXPORT 
#    endif
#  endif

#  ifndef CDT_NO_EXPORT
#    define CDT_NO_EXPORT 
#  endif
#endif

#ifndef CDT_DEPRECATED
#  define CDT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef CDT_DEPRECATED_EXPORT
#  define CDT_DEPRECATED_EXPORT CDT_EXPORT CDT_DEPRECATED
#endif

#ifndef CDT_DEPRECATED_NO_EXPORT
#  define CDT_DEPRECATED_NO_EXPORT CDT_NO_EXPORT CDT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CDT_NO_DEPRECATED
#    define CDT_NO_DEPRECATED
#  endif
#endif

#endif
