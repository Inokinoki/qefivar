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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_ACPI ||
        dp->subtype != QEFIDevicePathACPISubType::ACPI_HID)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32) + sizeof(quint32))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_ACPI ||
        dp->subtype != QEFIDevicePathACPISubType::ACPI_HIDEX)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32) + sizeof(quint32) + sizeof(quint32))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_ACPI ||
        dp->subtype != QEFIDevicePathACPISubType::ACPI_ADR)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    QList<quint32> addresses;
    for (int i = sizeof(struct qefi_device_path_header); i < length;
        i += sizeof(quint32)) {
        addresses << 
            qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
        dp_inner_pointer += sizeof(quint32);
    }
    return new QEFIDevicePathACPIADR(addresses);
}


// ACPI formating
QByteArray qefi_format_dp_acpi_hid(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_ACPI ||
        dp->subType() != QEFIDevicePathACPISubType::ACPI_HID)
        return QByteArray();
    QEFIDevicePathACPIHID *dp_instance =
        dynamic_cast<QEFIDevicePathACPIHID *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 hid =
        qToLittleEndian<quint32>(dp_instance->hid());
    buffer.append((const char *)&hid, sizeof(quint32));
    quint32 uid =
        qToLittleEndian<quint32>(dp_instance->uid());
    buffer.append((const char *)&uid, sizeof(quint32));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_acpi_hidex(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_ACPI ||
        dp->subType() != QEFIDevicePathACPISubType::ACPI_HIDEX)
        return QByteArray();
    QEFIDevicePathACPIHIDEX *dp_instance =
        dynamic_cast<QEFIDevicePathACPIHIDEX *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 hid =
        qToLittleEndian<quint32>(dp_instance->hid());
    buffer.append((const char *)&hid, sizeof(quint32));
    quint32 uid =
        qToLittleEndian<quint32>(dp_instance->uid());
    buffer.append((const char *)&uid, sizeof(quint32));
    quint32 cid =
        qToLittleEndian<quint32>(dp_instance->cid());
    buffer.append((const char *)&cid, sizeof(quint32));
    // TODO: Clarify the string encoding
    buffer.append(dp_instance->hidString().toUtf8());
    buffer.append(dp_instance->uidString().toUtf8());
    buffer.append(dp_instance->cidString().toUtf8());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_acpi_adr(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_ACPI ||
        dp->subType() != QEFIDevicePathACPISubType::ACPI_ADR)
        return QByteArray();
    QEFIDevicePathACPIADR *dp_instance =
        dynamic_cast<QEFIDevicePathACPIADR *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    const auto &addresses = dp_instance->addresses();
    for (const auto &addr : addresses) {
        quint32 adr = qToLittleEndian<quint32>(addr);
        buffer.append((const char *)&adr, sizeof(quint32));
    }

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
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
