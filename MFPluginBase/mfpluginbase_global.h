#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(MFPLUGINBASE_LIB)
#  define MFPLUGINBASE_EXPORT Q_DECL_EXPORT
# else
#  define MFPLUGINBASE_EXPORT Q_DECL_IMPORT
# endif
#else
# define MFPLUGINBASE_EXPORT
#endif
