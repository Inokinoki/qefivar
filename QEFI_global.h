#ifndef QEFI_GLOBAL_H
#define QEFI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QEFI_LIBRARY)
#  define QEFI_EXPORT Q_DECL_EXPORT
#else
#  define QEFI_EXPORT Q_DECL_IMPORT
#endif

#endif // QEFI_GLOBAL_H
