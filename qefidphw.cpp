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

// Hardware parsing
QEFIDevicePath *qefi_parse_dp_hardware_pci(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Hardware ||
        dp->subtype != QEFIDevicePathHardwareSubType::HW_PCI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    quint8 function = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 device = *dp_inner_pointer;
    return new QEFIDevicePathHardwarePCI(function, device);
}

QEFIDevicePath *qefi_parse_dp_hardware_pccard(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Hardware ||
        dp->subtype != QEFIDevicePathHardwareSubType::HW_PCCard)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    quint8 function = *dp_inner_pointer;
    return new QEFIDevicePathHardwarePCCard(function);
}

QEFIDevicePath *qefi_parse_dp_hardware_mmio(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Hardware ||
        dp->subtype != QEFIDevicePathHardwareSubType::HW_MMIO)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    QUuid vendorGuid = qefi_format_guid(dp_inner_pointer);
    quint32 memoryType = 
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 startingAddress =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 endingAddress =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathHardwareMMIO(memoryType,
        startingAddress, endingAddress);
}

QEFIDevicePath *qefi_parse_dp_hardware_vendor(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Hardware ||
        dp->subtype != QEFIDevicePathHardwareSubType::HW_Vendor)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    QUuid vendorGuid = qefi_format_guid(dp_inner_pointer);
    dp_inner_pointer += 16;
    QByteArray vendorData((char *)dp_inner_pointer, length - (dp_inner_pointer - (quint8 *)dp));
    return new QEFIDevicePathHardwareVendor(vendorGuid, vendorData);
}

QEFIDevicePath *qefi_parse_dp_hardware_controller(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Hardware ||
        dp->subtype != QEFIDevicePathHardwareSubType::HW_Controller)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    quint32 controller =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    return new QEFIDevicePathHardwareController(controller);
}

QEFIDevicePath *qefi_parse_dp_hardware_bmc(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Hardware ||
        dp->subtype != QEFIDevicePathHardwareSubType::HW_BMC)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) +
        sizeof(struct qefi_device_path_header);
    quint8 interfaceType = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint64 baseAddress =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathHardwareBMC(interfaceType, baseAddress);
}


// Hardware formating
QByteArray qefi_format_dp_hardware_pci(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_hardware_pccard(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_hardware_mmio(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_hardware_vendor(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_hardware_controller(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_hardware_bmc(QEFIDevicePath *dp)
{
    return QByteArray();
}


// Subclasses for hardware
QEFIDevicePathHardwarePCI::QEFIDevicePathHardwarePCI(quint8 function,
    quint8 device)
    : QEFIDevicePathHardware((quint8)QEFIDevicePathHardwareSubType::HW_PCI),
    m_function(function), m_device(device) {}

QEFIDevicePathHardwarePCCard::QEFIDevicePathHardwarePCCard(
    quint8 function)
    : QEFIDevicePathHardware((quint8)QEFIDevicePathHardwareSubType::HW_PCCard),
    m_function(function) {}

QEFIDevicePathHardwareMMIO::QEFIDevicePathHardwareMMIO(quint32 memoryType,
    quint64 startingAddress, quint64 endingAddress)
    : QEFIDevicePathHardware((quint8)QEFIDevicePathHardwareSubType::HW_MMIO),
    m_memoryType(memoryType),
    m_startingAddress(startingAddress), m_endingAddress(endingAddress) {}

QEFIDevicePathHardwareVendor::QEFIDevicePathHardwareVendor(
    QUuid vendorGuid, QByteArray vendorData)
    : QEFIDevicePathHardware((quint8)QEFIDevicePathHardwareSubType::HW_Vendor),
    m_vendorGuid(vendorGuid), m_vendorData(vendorData) {}

QEFIDevicePathHardwareController::QEFIDevicePathHardwareController(
    quint32 controller)
    : QEFIDevicePathHardware((quint8)QEFIDevicePathHardwareSubType::HW_Controller),
    m_controller(controller) {}

QEFIDevicePathHardwareBMC::QEFIDevicePathHardwareBMC(quint8 interfaceType,
    quint64 baseAddress)
    : QEFIDevicePathHardware((quint8)QEFIDevicePathHardwareSubType::HW_BMC),
    m_interfaceType(interfaceType), m_baseAddress(baseAddress) {}
