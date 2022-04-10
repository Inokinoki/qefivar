#include "qefi.h"

#include <QtEndian>
#include <QDebug>

#pragma pack(push, 1)
struct qefi_load_option_header {
    quint32 attributes;
    quint16 path_list_length;
};
#pragma pack(pop)

/* EFI device path header */
#pragma pack(push, 1)
struct qefi_device_path_header {
    quint8 type;
    quint8 subtype;
    quint16 length;
};
#pragma pack(pop)

// Utilities in qefi.cpp
int qefi_dp_length(const struct qefi_device_path_header *dp_header);
int qefi_dp_count(struct qefi_device_path_header *dp_header_pointer,
    int max_dp_size);
int qefi_dp_total_size(struct qefi_device_path_header *dp_header_pointer,
    int max_dp_size);
QString qefi_parse_ucs2_string(quint8 *data, int max_size);

// ACPI parsing
QEFIDevicePath *qefi_parse_dp_acpi_hid(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_ACPI ||
        dp->subtype != QEFIDevicePathACPISubType::ACPI_HID)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    quint32 hid =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint32 uid =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    return new QEFIDevicePathACPIHID(hid, uid);
}

QEFIDevicePath *qefi_parse_dp_acpi_hidex(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_ACPI ||
        dp->subtype != QEFIDevicePathACPISubType::ACPI_HID)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    quint32 hid =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint32 uid =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint32 cid =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    // TODO: Parse strings, the string format is not clear
    return new QEFIDevicePathACPIHIDEX(hid, uid, cid,
        QString(), QString(), QString());
}

QEFIDevicePath *qefi_parse_dp_acpi_adr(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_ACPI ||
        dp->subtype != QEFIDevicePathACPISubType::ACPI_ADR)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    QList<quint32> addresses;
    for (int i = sizeof(struct qefi_device_path_header); i < length;
        i += sizeof(quint32)) {
        addresses << 
            qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    }
    return new QEFIDevicePathACPIADR(addresses);
}


// Subclasses for ACPI
QEFIDevicePathACPIHID::QEFIDevicePathACPIHID(quint32 hid, quint32 uid)
    : QEFIDevicePathACPI((quint8)
        QEFIDevicePathACPISubType::ACPI_HID),
    m_hid(hid), m_uid(uid) {}

QEFIDevicePathACPIHIDEX::QEFIDevicePathACPIHIDEX(
    quint32 hid, quint32 uid, quint32 cid,
    QString hidString, QString uidString, QString cidString)
    : QEFIDevicePathACPI((quint8)
        QEFIDevicePathACPISubType::ACPI_HIDEX),
    m_hid(hid), m_uid(uid), m_cid(cid),
    m_hidString(hidString), m_uidString(uidString),
    m_cidString(cidString) {}

QEFIDevicePathACPIADR::QEFIDevicePathACPIADR(QList<quint32> addresses)
    : QEFIDevicePathACPI((quint8)QEFIDevicePathACPISubType::ACPI_ADR),
    m_addresses(addresses) {}
