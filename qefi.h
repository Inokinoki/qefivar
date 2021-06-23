#ifndef QEFI_H
#define QEFI_H

#include "QEFI_global.h"

#include <QUuid>

QEFI_EXPORT quint16 qefi_get_variable_uint16(QUuid uuid, QString name);
// QEFI_EXPORT QString qefi_get_variable_string(QUuid uuid, QString name);
QEFI_EXPORT QByteArray qefi_get_variable(QUuid uuid, QString name);

QEFI_EXPORT void qefi_set_variable_uint16(QUuid uuid, QString name, quint16 value);
// QEFI_EXPORT void qefi_set_variable_string(QUuid uuid, QString name, QString value);
QEFI_EXPORT void qefi_set_variable(QUuid uuid, QString name, QByteArray value);

#endif // QEFI_H
