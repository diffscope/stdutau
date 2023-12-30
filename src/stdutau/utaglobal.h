#ifndef UTAGLOBAL_H
#define UTAGLOBAL_H

#include <limits>

#ifdef _MSC_VER
#  define STDUTAU_DECL_EXPORT __declspec(dllexport)
#  define STDUTAU_DECL_IMPORT __declspec(dllimport)
#else
#  define STDUTAU_DECL_EXPORT __attribute__((visibility("default")))
#  define STDUTAU_DECL_IMPORT __attribute__((visibility("default")))
#endif

#ifndef STDUTAU_EXPORT
#  ifdef STDUTAU_STATIC
#    define STDUTAU_EXPORT
#  else
#    ifdef STDUTAU_LIBRARY
#      define STDUTAU_EXPORT STDUTAU_DECL_EXPORT
#    else
#      define STDUTAU_EXPORT STDUTAU_DECL_IMPORT
#    endif
#  endif
#endif

namespace Utau {

    constexpr const char NODEF_STRING[] = "%NODEF%";
    constexpr const int NODEF_INT = std::numeric_limits<int>::min();
    constexpr const double NODEF_DOUBLE = -1e+08;

    constexpr const char COMMA = ',';
    constexpr const char SIMICOLON = ';';
    constexpr const char EQUAL = '=';
    constexpr const char SLASH = '/';

}

#endif // UTAGLOBAL_H
