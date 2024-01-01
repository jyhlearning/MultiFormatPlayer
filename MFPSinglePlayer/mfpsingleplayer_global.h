#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(MFPSINGLEPLAYER_LIB)
#  define MFPSINGLEPLAYER_EXPORT Q_DECL_EXPORT
# else
#  define MFPSINGLEPLAYER_EXPORT Q_DECL_IMPORT
# endif
#else
# define MFPSINGLEPLAYER_EXPORT
#endif
