#ifndef QEFI_H
#define QEFI_H

#include <QtCore/qglobal.h>

#if defined(QEFI_LIBRARY)
#  define QEFI_EXPORT Q_DECL_EXPORT
#else
#  define QEFI_EXPORT Q_DECL_IMPORT
#endif

#include <QUuid>

QEFI_EXPORT bool qefi_is_available();

QEFI_EXPORT quint16 qefi_get_variable_uint16(QUuid uuid, QString name);
QEFI_EXPORT QByteArray qefi_get_variable(QUuid uuid, QString name);

QEFI_EXPORT void qefi_set_variable_uint16(QUuid uuid, QString name, quint16 value);
QEFI_EXPORT void qefi_set_variable(QUuid uuid, QString name, QByteArray value);

QEFI_EXPORT QString qefi_extract_name(QByteArray data);
QEFI_EXPORT QString qefi_extract_path(QByteArray data);

#endif // QEFI_H
