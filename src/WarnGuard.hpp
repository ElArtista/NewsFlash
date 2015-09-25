#ifndef _WARN_GUARD_HPP_
#define _WARN_GUARD_HPP_

#ifdef _MSC_VER
#define DO_PRAGMA(x) __pragma(x)
#else
#define DO_PRAGMA(x) _Pragma(#x)
#endif

//======================================================= 
// Clang
//======================================================= 
#define CLANG_WARN_GUARD_ON                         \
    DO_PRAGMA(clang diagnostic push)                \
    DO_PRAGMA(clang diagnostic ignored "-Wall")     \
    DO_PRAGMA(clang diagnostic ignored "-Wextra")

#define CLANG_WARN_GUARD_OFF                        \
    DO_PRAGMA(clang diagnostic pop)

//======================================================= 
// MSVC
//======================================================= 
#define MSVC_WARN_GUARD_ON                          \
    DO_PRAGMA(warning(push, 0))

#define MSVC_WARN_GUARD_OFF                         \
    DO_PRAGMA(warning(pop))

//======================================================= 
// Gcc
//======================================================= 
#define GCC_WARN_GUARD_ON                           \
    DO_PRAGMA(GCC diagnostic push)                  \
    DO_PRAGMA(GCC diagnostic ignored "-Wall")       \
    DO_PRAGMA(GCC diagnostic ignored "-Wextra")

#define GCC_WARN_GUARD_OFF                          \
    DO_PRAGMA(GCC diagnostic pop)

//======================================================= 
// Unknown
//======================================================= 
#define UNKNOWN_WARN_GUARD_ON
#define UNKNOWN_WARN_GUARD_OFF

//======================================================= 
// All
//======================================================= 

#ifdef __clang__
    #define WARN_GUARD_ON  CLANG_WARN_GUARD_ON
    #define WARN_GUARD_OFF CLANG_WARN_GUARD_OFF
#elif _MSC_VER
    #define WARN_GUARD_ON  MSVC_WARN_GUARD_ON
    #define WARN_GUARD_OFF MSVC_WARN_GUARD_OFF
#elif __GNUC__
    #define WARN_GUARD_ON  GCC_WARN_GUARD_ON
    #define WARN_GUARD_OFF GCC_WARN_GUARD_OFF
#else
    #define WARN_GUARD_ON  UNKNOWN_WARN_GUARD_ON
    #define WARN_GUARD_OFF UNKNOWN_WARN_GUARD_OFF
#endif

#endif
