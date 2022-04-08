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

int qefi_dp_length(const struct qefi_device_path_header *dp_header)
{
    if (!dp_header) return -1;
    return qFromLittleEndian<quint16>(dp_header->length);
}

int qefi_dp_count(struct qefi_device_path_header *dp_header_pointer, int max_dp_size)
{
    if (!dp_header_pointer) return -1;

    int count = 0;
    int size = 0;
    while (size < max_dp_size) {
        int tempLength = qefi_dp_length(dp_header_pointer);
        if (tempLength <= 0) return tempLength;

        size += tempLength;
        count++;

        if (dp_header_pointer->type == QEFIDevicePathType::DP_End &&
            dp_header_pointer->subtype == 0xFF)
            break;

        dp_header_pointer = (struct qefi_device_path_header *)
            ((quint8 *)dp_header_pointer + tempLength);
    }
    return count;
}

int qefi_dp_total_size(struct qefi_device_path_header *dp_header_pointer, int max_dp_size)
{
    if (!dp_header_pointer) return -1;

    if (dp_header_pointer->type == QEFIDevicePathType::DP_End &&
        dp_header_pointer->subtype == 0xFF) return qefi_dp_length(dp_header_pointer);

    int size = 0;
    int count = qefi_dp_count(dp_header_pointer, max_dp_size);
    if (count < 0) return -1;
    for (int i = 0; i < count, size < max_dp_size; i++) {
        int tempLength = qefi_dp_length(dp_header_pointer);
        if (tempLength <= 0) return tempLength;

        size += tempLength;

        if (dp_header_pointer->type == QEFIDevicePathType::DP_End &&
            dp_header_pointer->subtype == 0xFF)
            break;

        dp_header_pointer = (struct qefi_device_path_header *)
            ((quint8 *)dp_header_pointer + tempLength);
    }
    return (size == max_dp_size ? size : -1);
}

QString qefi_parse_ucs2_string(quint8 *data, int max_size)
{
    QString str; str.reserve(max_size / 2);
    quint16 *c = (quint16 *)data;
    for (int index = 0; index < max_size; index += 2, c++) {
        if (*c == 0) break;
        str.append(QChar(qFromLittleEndian<quint16>(*c)));
    }
    return str;
}

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

// Message parsing
QEFIDevicePath *qefi_parse_dp_message_atapi(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_ATAPI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 primary = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 slave = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint16 lun =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    return new QEFIDevicePathMessageATAPI(primary, slave, lun);
}

QEFIDevicePath *qefi_parse_dp_message_scsi(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SCSI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 target =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 lun =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    return new QEFIDevicePathMessageSCSI(target, lun);
}

QEFIDevicePath *qefi_parse_dp_message_fibre_chan(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_FibreChan)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 reserved =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 wwn =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 lun =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathMessageFibreChan(reserved, wwn, lun);
}

QEFIDevicePath *qefi_parse_dp_message_1394(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_1394)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 reversed =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 guid =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathMessage1394(reversed, guid);
}

QEFIDevicePath *qefi_parse_dp_message_usb(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_USB)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 parentPort = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 inter = *dp_inner_pointer;
    return new QEFIDevicePathMessageUSB(parentPort, inter);
}

QEFIDevicePath *qefi_parse_dp_message_i2o(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_I2O)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 target =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    return new QEFIDevicePathMessageI2O(target);
}

// TODO: Add inifiband class

QEFIDevicePath *qefi_parse_dp_message_vendor(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_Vendor)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUuid vendorGuid = qefi_format_guid(dp_inner_pointer);
    dp_inner_pointer += 16 * sizeof(quint8);
    QByteArray vendorData((char *)dp_inner_pointer,
        length - (dp_inner_pointer - (quint8 *)dp));
    return new QEFIDevicePathMessageVendor(vendorGuid, vendorData);
}

QEFIDevicePath *qefi_parse_dp_message_mac_addr(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_MACAddr)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 *macAddress = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 32;
    quint8 interfaceType = *dp_inner_pointer;
    return new QEFIDevicePathMessageMACAddr(macAddress, interfaceType);
}

QEFIDevicePath *qefi_parse_dp_message_ipv4(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_IPv4)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 *localIPv4Addr = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 4;
    quint8 *remoteIPv4Addr = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 4;
    quint16 localPort =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 remotePort =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 protocol =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint8 staticIPAddr = *dp_inner_pointer;    // A bool value
    dp_inner_pointer += sizeof(quint8);
    quint8 *gateway = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 4;
    quint8 *netmask = dp_inner_pointer;
    return new QEFIDevicePathMessageIPv4Addr(
        localIPv4Addr, remoteIPv4Addr,
        localPort, remotePort,
        protocol, staticIPAddr,
        gateway, netmask
    );
}

QEFIDevicePath *qefi_parse_dp_message_ipv6(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_IPv6)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 *localIPv6Addr = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 16;
    quint8 *remoteIPv6Addr = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 16;
    quint16 localPort =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 remotePort =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 protocol =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint8 ipAddrOrigin = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 prefixLength = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 gatewayIPv6Addr = *dp_inner_pointer;
    return new QEFIDevicePathMessageIPv6Addr(
        localIPv6Addr, remoteIPv6Addr,
        localPort, remotePort, protocol,
        ipAddrOrigin, prefixLength, gatewayIPv6Addr
    );
}

QEFIDevicePath *qefi_parse_dp_message_uart(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_UART)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 reserved =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 baudRate =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));;
    dp_inner_pointer += sizeof(quint64);
    quint8 dataBits = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 parity = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 stopBits = *dp_inner_pointer;
    return new QEFIDevicePathMessageUART(reserved,
        baudRate, dataBits, parity, stopBits);
}

QEFIDevicePath *qefi_parse_dp_message_usb_class(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_USBClass)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 vendorId =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 productId =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint8 deviceClass = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 deviceSubclass = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 deviceProtocol = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    return new QEFIDevicePathMessageUSBClass(vendorId, productId,
        deviceClass, deviceSubclass, deviceProtocol);
}

QEFIDevicePath *qefi_parse_dp_message_usb_wwid(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_USBWWID)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 vendorId =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 productId =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    // TODO: Parse sn
    quint16 *sn = (quint16 *)dp_inner_pointer;
    return new QEFIDevicePathMessageUSBWWID(vendorId, productId, sn);
}

QEFIDevicePath *qefi_parse_dp_message_lun(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_LUN)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 lun = *dp_inner_pointer;
    return new QEFIDevicePathMessageLUN(lun);
}

QEFIDevicePath *qefi_parse_dp_message_sata(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SATA)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 hbaPort =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 portMultiplierPort =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint8 lun = *dp_inner_pointer;
    return new QEFIDevicePathMessageSATA(hbaPort, portMultiplierPort, lun);
}

QEFIDevicePath *qefi_parse_dp_message_iscsi(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_ISCSI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 protocol =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint16 options =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    quint8 *lun = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 8;
    quint16 tpgt =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint16);
    QString targetName(QByteArray((const char *)dp_inner_pointer,
        length - (dp_inner_pointer - (quint8 *)dp)));
    return new QEFIDevicePathMessageISCSI(protocol, options, lun,
        tpgt, targetName);
}

QEFIDevicePath *qefi_parse_dp_message_vlan(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_VLAN)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 vlanID =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    return new QEFIDevicePathMessageVLAN(vlanID);
}

QEFIDevicePath *qefi_parse_dp_message_fibre_chan_ex(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_FibreChanEx)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 reserved =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint8 *wwn = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 8;
    quint8 *lun = dp_inner_pointer;
    return new QEFIDevicePathMessageFibreChanEx(reserved, wwn, lun);
}

QEFIDevicePath *qefi_parse_dp_message_sas_ex(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SASEX)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 vlanID =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    quint8 *sasAddress = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 8;
    quint8 *lun = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 8;
    quint8 deviceTopologyInfo = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 driveBayID = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint16 rtp =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    return new QEFIDevicePathMessageSASEx(sasAddress, lun,
        deviceTopologyInfo, driveBayID, rtp);
}

QEFIDevicePath *qefi_parse_dp_message_nvme(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_NVME)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 nid =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    return new QEFIDevicePathMessageNVME(nid, dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_uri(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_URI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUrl uri(QString(QByteArray((const char *)dp_inner_pointer,
        length - (dp_inner_pointer - (quint8 *)dp))));
    return new QEFIDevicePathMessageURI(uri);
}

QEFIDevicePath *qefi_parse_dp_message_ufs(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_UFS)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 targetID = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 lun = *dp_inner_pointer;
    return new QEFIDevicePathMessageUFS(targetID, lun);
}

QEFIDevicePath *qefi_parse_dp_message_sd(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SD)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMessageSD(*dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_bt(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_BT)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMessageBT(dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_wifi(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_WiFi)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    int ssid_len = length - sizeof(struct qefi_device_path_header);
    QString ssid(QByteArray((const char *)dp_inner_pointer,
        ssid_len < 32 ? ssid_len : 32));
    return new QEFIDevicePathMessageWiFi(ssid);
}

QEFIDevicePath *qefi_parse_dp_message_emmc(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_EMMC)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMessageEMMC(*dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_btle(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_BTLE)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 *address = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 6;
    quint8 addressType = *dp_inner_pointer;
    return new QEFIDevicePathMessageBTLE(address, addressType);
}

QEFIDevicePath *qefi_parse_dp_message_dns(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_DNS)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 is_ipv6 = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    // TODO: Parse addresses
    return new QEFIDevicePathMessageDNS();
}

QEFIDevicePath *qefi_parse_dp_message_nvdimm(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_NVDIMM)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUuid uuid = qefi_format_guid(dp_inner_pointer);
    return new QEFIDevicePathMessageNVDIMM(uuid);
}

// Media parsing
QEFIDevicePath *qefi_parse_dp_media_file(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_File)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMediaFile(qefi_parse_ucs2_string(dp_inner_pointer, 
        length - sizeof(struct qefi_device_path_header)));
}

QEFIDevicePath *qefi_parse_dp_media_hdd(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_HD)
        return nullptr;
	int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = (quint8 *)dp + sizeof(struct qefi_device_path_header);
    quint32 partitionNumber =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 start =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 size =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint8 *signature = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 16;
    quint8 format = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 signatureType = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    return new QEFIDevicePathMediaHD(partitionNumber,
        start, size, signature, format, signatureType);
}

QEFIDevicePath *qefi_parse_dp_media_cdrom(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_CDROM)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 entry =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 partitionRba =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 sectors =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathMediaCDROM(entry, partitionRba, sectors);
}

QEFIDevicePath *qefi_parse_dp_media_vendor(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_Vendor)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUuid vendorGuid = qefi_format_guid(dp_inner_pointer);
    dp_inner_pointer += 16 * sizeof(quint8);
    QByteArray vendorData((char *)dp_inner_pointer, length - (dp_inner_pointer - (quint8 *)dp));
    return new QEFIDevicePathMediaVendor(vendorGuid, vendorData);
}

QEFIDevicePath *qefi_parse_dp_media_protocol(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_Protocol)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUuid protocolGuid = qefi_format_guid(dp_inner_pointer);
    return new QEFIDevicePathMediaProtocol(protocolGuid);
}

QEFIDevicePath *qefi_parse_dp_media_firmware_file(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_FirmwareFile)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QByteArray piInfo((char *)dp_inner_pointer, length);
    return new QEFIDevicePathMediaFirmwareFile(piInfo);
}

QEFIDevicePath *qefi_parse_dp_media_fv(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QByteArray piInfo((char *)dp_inner_pointer, length);
    return new QEFIDevicePathMediaFirmwareFile(piInfo);
}

QEFIDevicePath *qefi_parse_dp_media_relative_offset(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_RelativeOffset)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 reserved =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 firstByte =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 lastByte =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathMediaRelativeOffset(reserved, firstByte, lastByte);
}

QEFIDevicePath *qefi_parse_dp_media_ramdisk(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < 4) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media ||
        dp->subtype != QEFIDevicePathMediaSubType::MEDIA_RamDisk)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // TODO: Check size
    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint64 startAddress =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 endAddress =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    QUuid diskTypeGuid = qefi_format_guid(dp_inner_pointer);
    dp_inner_pointer += sizeof(quint8) * 16;
    quint16 instanceNumber =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    return new QEFIDevicePathMediaRAMDisk(startAddress,
        endAddress, diskTypeGuid, instanceNumber);
}

QEFIDevicePath *qefi_private_parse_message_subtype(struct qefi_device_path_header *dp, int dp_size)
{
	quint8 type = dp->type, subtype = dp->subtype;
    if (type != QEFIDevicePathType::DP_Message) return nullptr;
    int length = dp_size;
    if (length <= 0) return nullptr;

    switch (subtype) {
        case QEFIDevicePathMessageSubType::MSG_ATAPI:
            qDebug() << "Parsing DP message ATAPI";
            return qefi_parse_dp_message_atapi(dp, length);
        case QEFIDevicePathMessageSubType::MSG_SCSI:
            qDebug() << "Parsing DP message SCSI";
            return qefi_parse_dp_message_scsi(dp, length);
        case QEFIDevicePathMessageSubType::MSG_FibreChan:
            qDebug() << "Parsing DP message FibreChan";
            return qefi_parse_dp_message_fibre_chan(dp, length);
        case QEFIDevicePathMessageSubType::MSG_1394:
            qDebug() << "Parsing DP message 1394";
            return qefi_parse_dp_message_1394(dp, length);
        case QEFIDevicePathMessageSubType::MSG_USB:
            qDebug() << "Parsing DP message USB";
            return qefi_parse_dp_message_usb(dp, length);
        case QEFIDevicePathMessageSubType::MSG_I2O:
            qDebug() << "Parsing DP message I2O";
            return qefi_parse_dp_message_i2o(dp, length);
        case QEFIDevicePathMessageSubType::MSG_InfiniBand:
            // TODO: Add inifiband class
        case QEFIDevicePathMessageSubType::MSG_Vendor:
            qDebug() << "Parsing DP message Vendor";
            return qefi_parse_dp_message_vendor(dp, length);
        case QEFIDevicePathMessageSubType::MSG_MACAddr:
            qDebug() << "Parsing DP message Mac Address";
            return qefi_parse_dp_message_mac_addr(dp, length);
        case QEFIDevicePathMessageSubType::MSG_IPv4:
            qDebug() << "Parsing DP message IPv4";
            return qefi_parse_dp_message_ipv4(dp, length);
        case QEFIDevicePathMessageSubType::MSG_IPv6:
            qDebug() << "Parsing DP message IPv6";
            return qefi_parse_dp_message_ipv6(dp, length);
        case QEFIDevicePathMessageSubType::MSG_UART:
            qDebug() << "Parsing DP message UART";
            return qefi_parse_dp_message_uart(dp, length);
        case QEFIDevicePathMessageSubType::MSG_USBClass:
            qDebug() << "Parsing DP message USB Class";
            return qefi_parse_dp_message_usb_class(dp, length);

        case QEFIDevicePathMessageSubType::MSG_USBWWID:
            qDebug() << "Parsing DP message USBWWID";
            return qefi_parse_dp_message_usb_wwid(dp, length);

        case QEFIDevicePathMessageSubType::MSG_LUN:
            qDebug() << "Parsing DP message LUN";
            return qefi_parse_dp_message_lun(dp, length);
        case QEFIDevicePathMessageSubType::MSG_SATA:
            qDebug() << "Parsing DP message SATA";
            return qefi_parse_dp_message_sata(dp, length);
        case QEFIDevicePathMessageSubType::MSG_ISCSI:
            qDebug() << "Parsing DP message ISCSI";
            return qefi_parse_dp_message_iscsi(dp, length);
        case QEFIDevicePathMessageSubType::MSG_VLAN:
            qDebug() << "Parsing DP message VLAN";
            return qefi_parse_dp_message_vlan(dp, length);

        case QEFIDevicePathMessageSubType::MSG_FibreChanEx:
            qDebug() << "Parsing DP message FibreChanEx";
            return qefi_parse_dp_message_fibre_chan_ex(dp, length);
        case QEFIDevicePathMessageSubType::MSG_SASEX:
            qDebug() << "Parsing DP message SASEX";
            return qefi_parse_dp_message_sas_ex(dp, length);

        case QEFIDevicePathMessageSubType::MSG_NVME:
            qDebug() << "Parsing DP message NVME";
            return qefi_parse_dp_message_nvme(dp, length);
        case QEFIDevicePathMessageSubType::MSG_URI:
            qDebug() << "Parsing DP message URI";
            return qefi_parse_dp_message_uri(dp, length);
        case QEFIDevicePathMessageSubType::MSG_UFS:
            qDebug() << "Parsing DP message UFS";
            return qefi_parse_dp_message_ufs(dp, length);
        case QEFIDevicePathMessageSubType::MSG_SD:
            qDebug() << "Parsing DP message SD";
            return qefi_parse_dp_message_sd(dp, length);
        case QEFIDevicePathMessageSubType::MSG_BT:
            qDebug() << "Parsing DP message Bluetooth";
            return qefi_parse_dp_message_bt(dp, length);
        case QEFIDevicePathMessageSubType::MSG_WiFi:
            qDebug() << "Parsing DP message WiFi";
            return qefi_parse_dp_message_wifi(dp, length);
        case QEFIDevicePathMessageSubType::MSG_EMMC:
            qDebug() << "Parsing DP message EMMC";
            return qefi_parse_dp_message_emmc(dp, length);
        case QEFIDevicePathMessageSubType::MSG_BTLE:
            qDebug() << "Parsing DP message BTLE";
            return qefi_parse_dp_message_btle(dp, length);
        case QEFIDevicePathMessageSubType::MSG_DNS:
            qDebug() << "Parsing DP message DNS";
            return qefi_parse_dp_message_dns(dp, length);
        case QEFIDevicePathMessageSubType::MSG_NVDIMM:
            qDebug() << "Parsing DP message NVDIMM";
            return qefi_parse_dp_message_nvdimm(dp, length);
    }
    return nullptr;
}

QEFIDevicePath *qefi_parse_dp(struct qefi_device_path_header *dp, int dp_size)
{
	quint8 type = dp->type, subtype = dp->subtype;
	int length = qefi_dp_length(dp);
    qDebug() << "Parsing DP: length " << length << " " <<
        "type" << type << "subtype" << subtype;
    if (length != dp_size || length <= 0) return nullptr;

    if (type == QEFIDevicePathType::DP_Hardware) {
        // Parse hardware
        switch (subtype) {
            case QEFIDevicePathHardwareSubType::HW_PCI:
                qDebug() << "Parsing DP hardware PCI";
                return qefi_parse_dp_hardware_pci(dp, length);
            case QEFIDevicePathHardwareSubType::HW_PCCard:
                qDebug() << "Parsing DP hardware PCCard";
                return qefi_parse_dp_hardware_pccard(dp, length);
            case QEFIDevicePathHardwareSubType::HW_MMIO:
                qDebug() << "Parsing DP hardware MMIO";
                return qefi_parse_dp_hardware_mmio(dp, length);
            case QEFIDevicePathHardwareSubType::HW_Vendor:
                qDebug() << "Parsing DP hardware Vendor";
                return qefi_parse_dp_hardware_vendor(dp, length);
            case QEFIDevicePathHardwareSubType::HW_Controller:
                qDebug() << "Parsing DP hardware Controller";
                return qefi_parse_dp_hardware_controller(dp, length);
            case QEFIDevicePathHardwareSubType::HW_BMC:
                qDebug() << "Parsing DP hardware BMC";
                return qefi_parse_dp_hardware_bmc(dp, length);
        }
    } else if (type == QEFIDevicePathType::DP_ACPI) {
        // Parse DP_ACPI
        switch (subtype) {
            case QEFIDevicePathACPISubType::ACPI_HID:
                qDebug() << "Parsing DP ACPI HID";
                return qefi_parse_dp_acpi_hid(dp, length);
            case QEFIDevicePathACPISubType::ACPI_HIDEX:
                qDebug() << "Parsing DP ACPI HIDEX";
                return qefi_parse_dp_acpi_hidex(dp, length);
            case QEFIDevicePathACPISubType::ACPI_ADR:
                qDebug() << "Parsing DP ACPI ADR";
                return qefi_parse_dp_acpi_adr(dp, length);
        }
    } else if (type == QEFIDevicePathType::DP_Message) {
        // Parse Message
        return qefi_private_parse_message_subtype(dp, length);
    } else if (type == QEFIDevicePathType::DP_Media) {
        // Parse Media
        switch (subtype) {
            case QEFIDevicePathMediaSubType::MEDIA_HD:
                qDebug() << "Parsing DP media HD";
                return qefi_parse_dp_media_hdd(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_File:
                qDebug() << "Parsing DP media file";
                return qefi_parse_dp_media_file(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_CDROM:
                qDebug() << "Parsing DP media CDROM";
                return qefi_parse_dp_media_cdrom(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_Vendor:
                qDebug() << "Parsing DP media vendor";
                return qefi_parse_dp_media_vendor(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_Protocol:
                qDebug() << "Parsing DP media protocol";
                return qefi_parse_dp_media_protocol(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_FirmwareFile:
                qDebug() << "Parsing DP media firmware file";
                return qefi_parse_dp_media_firmware_file(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume:
                qDebug() << "Parsing DP media FV";
                return qefi_parse_dp_media_fv(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_RelativeOffset:
                qDebug() << "Parsing DP media relative offset";
                return qefi_parse_dp_media_relative_offset(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_RamDisk:
                qDebug() << "Parsing DP media ramdisk";
                return qefi_parse_dp_media_ramdisk(dp, length);
        }
    } else if (type == QEFIDevicePathType::DP_BIOSBoot) {
        // Parse BIOSBoot
        quint8 *dp_inner_pointer = (quint8 *)dp + sizeof(struct qefi_device_path_header);
        quint16 deviceType =
            qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
        dp_inner_pointer += sizeof(quint16);
        quint16 status =
            qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
        dp_inner_pointer += sizeof(quint16);
        QByteArray description; // TODO: Parse it
        return new QEFIDevicePathBIOSBoot(deviceType, status, description);
    }
    return nullptr;
}

QByteArray qefi_format_dp(QEFIDevicePath *dp)
{
	QEFIDevicePathType type = dp->type();
    quint8 subtype = dp->subType();
    qDebug() << "Formating DP: type" << type << "subtype" << subtype;

    if (type == QEFIDevicePathType::DP_Hardware) {
        // TODO: Format hardware
    } else if (type == QEFIDevicePathType::DP_ACPI) {
        // TODO: Format DP_ACPI
    } else if (type == QEFIDevicePathType::DP_Message) {
        // TODO: Format Message)
    } else if (type == QEFIDevicePathType::DP_Media) {
        // Format Media
        switch (subtype) {
            case QEFIDevicePathMediaSubType::MEDIA_HD:
                qDebug() << "Formating DP media HD";
                // TODO
            case QEFIDevicePathMediaSubType::MEDIA_File:
                qDebug() << "Formating DP media file";
                // TODO
        }
    } else if (type == QEFIDevicePathType::DP_BIOSBoot) {
        // TODO: Format BIOSBoot
    }
    return QByteArray();
}

#ifdef Q_OS_WIN
/* Implementation based on Windows API */
#include <Windows.h>
#include <WinBase.h>
#include <tchar.h>
#include <wbemidl.h>

DWORD ObtainPrivileges(LPCTSTR privilege) {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    BOOL res;
    // Obtain required privileges
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        return GetLastError();
    }

    res = LookupPrivilegeValue(NULL, privilege, &tkp.Privileges[0].Luid);
    if (!res)
    {
        return GetLastError();
    }
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    return GetLastError();
}

bool qefi_is_available()
{
    FIRMWARE_TYPE fType;
    BOOL status = GetFirmwareType(&fType);
    return status && fType == FirmwareTypeUefi;
}

bool qefi_has_privilege()
{
    return ObtainPrivileges(SE_SYSTEM_ENVIRONMENT_NAME) == ERROR_SUCCESS;
}

DWORD read_efivar_win(LPCTSTR name, LPCTSTR uuid, PVOID buffer, DWORD size)
{
    DWORD len = GetFirmwareEnvironmentVariable(name, uuid, buffer, size);
    if (len == 0)
    {
        DWORD errorCode = GetLastError();
        // std::cerr << "Error reading : code " << errorCode << "\n";
    }
    return len;
}

DWORD write_efivar_win(LPCTSTR name, LPCTSTR uuid, PVOID buffer, DWORD size)
{
    DWORD len = SetFirmwareEnvironmentVariable(name, uuid, buffer, size);
    if (len == 0)
    {
        DWORD errorCode = GetLastError();
        // std::cerr << "Error writing : code " << errorCode << "\n";
    }
    return len;
}

#define EFIVAR_BUFFER_SIZE 4096

quint16 qefi_get_variable_uint16(QUuid uuid, QString name)
{
#ifdef UNICODE
    std::wstring std_name = name.toStdWString();
    std::wstring std_uuid = uuid.toString(QUuid::WithBraces).toStdWString();
#else
    std::string std_name = name.toStdString();
    std::string std_uuid = uuid.toString(QUuid::WithBraces).toStdString();
#endif
    LPCTSTR c_name = std_name.c_str();
    LPCTSTR c_uuid = std_uuid.c_str();

    // Create a buffer
    TCHAR buffer[EFIVAR_BUFFER_SIZE];

    size_t length = read_efivar_win(c_name, c_uuid, (PVOID)buffer, EFIVAR_BUFFER_SIZE);

    if (length < 2)
    {
        return 0;
    }

    // Read as uint16, platform-independant
    quint16 value = *((quint16 *)buffer);

    return qFromLittleEndian<quint16>(value);
}

QByteArray qefi_get_variable(QUuid uuid, QString name)
{
#ifdef UNICODE
    std::wstring std_name = name.toStdWString();
    std::wstring std_uuid = uuid.toString(QUuid::WithBraces).toStdWString();
#else
    std::string std_name = name.toStdString();
    std::string std_uuid = uuid.toString(QUuid::WithBraces).toStdString();
#endif
    LPCTSTR c_name = std_name.c_str();
    LPCTSTR c_uuid = std_uuid.c_str();

    // Create a buffer
    TCHAR buffer[EFIVAR_BUFFER_SIZE];

    size_t length = read_efivar_win(c_name, c_uuid, (PVOID)buffer, EFIVAR_BUFFER_SIZE);

    QByteArray value;
    if (length == 0)
    {
        value.clear();
    }
    else
    {
        for (size_t i = 0; i < length; i++) {
            value.append(buffer[i]);
        }
    }

    return value;
}

void qefi_set_variable_uint16(QUuid uuid, QString name, quint16 value)
{
#ifdef UNICODE
    std::wstring std_name = name.toStdWString();
    std::wstring std_uuid = uuid.toString(QUuid::WithBraces).toStdWString();
#else
    std::string std_name = name.toStdString();
    std::string std_uuid = uuid.toString(QUuid::WithBraces).toStdString();
#endif
    LPCTSTR c_name = std_name.c_str();
    LPCTSTR c_uuid = std_uuid.c_str();

    // Create a buffer
    TCHAR buffer[sizeof(quint16) / sizeof(TCHAR)];
    quint16 *p = (quint16 *)buffer;
    *p = qToLittleEndian<quint16>(value);

    write_efivar_win(c_name, c_uuid, (PVOID)buffer, 2);
}

void qefi_set_variable(QUuid uuid, QString name, QByteArray value)
{
#ifdef UNICODE
    std::wstring std_name = name.toStdWString();
    std::wstring std_uuid = uuid.toString(QUuid::WithBraces).toStdWString();
#else
    std::string std_name = name.toStdString();
    std::string std_uuid = uuid.toString(QUuid::WithBraces).toStdString();
#endif
    LPCTSTR c_name = std_name.c_str();
    LPCTSTR c_uuid = std_uuid.c_str();

    write_efivar_win(c_name, c_uuid, (PVOID)value.data(), value.size());
}

#else
/* Implementation based on libefivar */
extern "C" {
#ifndef EFIVAR_OLD_API
#include <efivar/efivar.h>
#include <efivar/efiboot.h>
#else
#include <efivar.h>
#endif
#include <unistd.h>
}

bool qefi_is_available()
{
    return efi_variables_supported();
}

bool qefi_has_privilege()
{
    if (getuid() != 0) return false;
    return true;
}

quint16 qefi_get_variable_uint16(QUuid uuid, QString name)
{
    int return_code;

    std::string std_name = name.toStdString();
    const char *c_name = std_name.c_str();
    std::string std_uuid = uuid.toString(QUuid::WithoutBraces).toStdString();
    const char *c_uuid = std_uuid.c_str();

    efi_guid_t guid;
    return_code = efi_str_to_guid(c_uuid, &guid);
    if (return_code != 0)
    {
        return 0;
    }

    size_t var_size;
    return_code = efi_get_variable_size(guid, c_name, &var_size);
    if (var_size == 0 || return_code != 0)
    {
        return 0;
    }

    uint8_t *data;
    uint32_t attributes;
    return_code = efi_get_variable(guid, c_name, &data, &var_size, &attributes);

    quint16 value;
#ifndef EFIVAR_OLD_API
    if (return_code != 0)
#else
    if (return_code == 0)
#endif
    {
        value = 0;
    }
    else
    {
        // Read as uint16, platform-independant
        value = *((quint16 *)data);
        free(data);
    }

    return qFromLittleEndian<quint16>(value);
}

QByteArray qefi_get_variable(QUuid uuid, QString name)
{
    int return_code;

    std::string std_name = name.toStdString();
    const char *c_name = std_name.c_str();
    std::string std_uuid = uuid.toString(QUuid::WithoutBraces).toStdString();
    const char *c_uuid = std_uuid.c_str();

    efi_guid_t guid;
    return_code = efi_str_to_guid(c_uuid, &guid);
    if (return_code != 0)
    {
        return QByteArray();
    }

    size_t var_size;
    return_code = efi_get_variable_size(guid, c_name, &var_size);
    if (var_size == 0 || return_code != 0)
    {
        return QByteArray();
    }

    uint8_t *data;
    uint32_t attributes;
    return_code = efi_get_variable(guid, c_name, &data, &var_size, &attributes);

    QByteArray value;
#ifndef EFIVAR_OLD_API
    if (return_code != 0)
#else
    if (return_code == 0)
#endif
    {
        value.clear();
    }
    else
    {
        for (size_t i = 0; i < var_size; i++) {
            value.append(data[i]);
        }
        free(data);
    }

    return value;
}


const uint32_t default_write_attribute = EFI_VARIABLE_NON_VOLATILE |
                                         EFI_VARIABLE_BOOTSERVICE_ACCESS |
                                         EFI_VARIABLE_RUNTIME_ACCESS;

void qefi_set_variable_uint16(QUuid uuid, QString name, quint16 value)
{
    int return_code;

    std::string std_name = name.toStdString();
    const char *c_name = std_name.c_str();
    std::string std_uuid = uuid.toString(QUuid::WithoutBraces).toStdString();
    const char *c_uuid = std_uuid.c_str();

    efi_guid_t guid;
    return_code = efi_str_to_guid(c_uuid, &guid);

#ifndef EFIVAR_OLD_API
    if (return_code != 0)
#else
    if (return_code == 0)
#endif
    {
        return;
    }

    uint8_t buffer[2];
    *((uint16_t *)buffer) = qToLittleEndian<quint16>(value);
    return_code = efi_set_variable(guid, c_name, buffer, 2,
                                   default_write_attribute
#ifndef EFIVAR_OLD_API
                                 , 0644
#endif
                                   );

    // TODO: Detect return code
}

void qefi_set_variable(QUuid uuid, QString name, QByteArray value)
{
    int return_code;

    std::string std_name = name.toStdString();
    const char *c_name = std_name.c_str();
    std::string std_uuid = uuid.toString(QUuid::WithoutBraces).toStdString();
    const char *c_uuid = std_uuid.c_str();

    efi_guid_t guid;
    return_code = efi_str_to_guid(c_uuid, &guid);
    if (return_code != 0)
    {
        return;
    }

    return_code = efi_set_variable(guid, c_name, (uint8_t *)value.data(), value.size(),
                                   default_write_attribute
#ifndef EFIVAR_OLD_API
                                 , 0644
#endif
                                   );

    // TODO: Detect return code
}
#endif

/* General functions */
QString qefi_extract_name(const QByteArray &data)
{
    QString entry_name;
    if (qefi_loadopt_is_valid(data)) {
        int desc_length = qefi_loadopt_description_length(data);
        if (desc_length < 0) return entry_name;

        return qefi_parse_ucs2_string((quint8 *)(data.data() +
            sizeof(struct qefi_load_option_header)), desc_length);
    }
    return entry_name;
}

QString qefi_extract_path(const QByteArray &data)
{
    QString path;
    if (qefi_loadopt_is_valid(data)) {
        int desc_length = qefi_loadopt_description_length(data);
        if (desc_length < 0) return path;
        desc_length += 2;

        int dp_list_length = qefi_loadopt_dp_list_length(data);
        if (dp_list_length < 0) return path;

        quint16 *c = (quint16*)(data.data() +
            sizeof(struct qefi_load_option_header) + desc_length);

        // Keep the remainder length
        qint32 remainder_length = dp_list_length;
        quint8 *list_pointer = ((quint8 *)c);
        while (remainder_length > 0) {
            struct qefi_device_path_header *dp_header =
                (struct qefi_device_path_header *)list_pointer;
            int length = qefi_dp_length(dp_header);
            if (length < 0) return path;

            if (dp_header->type == DP_Media && dp_header->subtype == MEDIA_File) {
                // Media File
                QScopedPointer<QEFIDevicePath> dp(
                    qefi_parse_dp_media_file(dp_header, length));
                if (dp.isNull()) continue;
                QEFIDevicePathMediaFile *media_file_dp =
                    dynamic_cast<QEFIDevicePathMediaFile *>(dp.get());
                if (media_file_dp == nullptr) continue;
                path.append(media_file_dp->name());
                break;
            } else if (dp_header->type == DP_End) {
                // End
                break;
            }
            list_pointer += length;
            remainder_length -= length;
        }
    }
    return path;
}

QByteArray qefi_extract_optional_data(const QByteArray &data)
{
    int optional_data_len = qefi_loadopt_optional_data_length(data);
    if (optional_data_len > 0 && optional_data_len < data.size()) {
        // The optional data lays on the tail of load option
        return QByteArray(data.constData() +
            (data.size() - optional_data_len), optional_data_len);
    }
    return QByteArray();
}

int qefi_loadopt_description_length(const QByteArray &data)
{
    int size = data.size();
    int tempLength;

    // Check header
    if (size < sizeof(struct qefi_load_option_header)) return -1;

    tempLength = qefi_loadopt_dp_list_length(data);
    if (tempLength < 0) return -1;
    quint16 dpListLength = (quint16)(tempLength & 0xFFFF);

    quint16 *c = (quint16*)(data.data() + sizeof(struct qefi_load_option_header));
    bool isDescValid = false;
    tempLength = 0;
    while (size > 0) {
        // Find the end of description
        if (*c == 0) {
            isDescValid = true;
            break;
        }
        size -= 2, c++, tempLength += 2;
    }
    if (!isDescValid) return -1;

    return tempLength;
}

int qefi_loadopt_dp_list_length(const QByteArray &data)
{
    int size = data.size();

    // Check header
    if (size < sizeof(struct qefi_load_option_header)) return -1;

    struct qefi_load_option_header *header =
        (struct qefi_load_option_header *)data.data();
    return qFromLittleEndian<quint16>(header->path_list_length);
}

int qefi_loadopt_optional_data_length(const QByteArray &data)
{
    int size = data.size();
    int tempLength;

    // Check header
    if (size < sizeof(struct qefi_load_option_header)) return -1;
    size -= sizeof(struct qefi_load_option_header);

    // Check device path list length
    tempLength = qefi_loadopt_dp_list_length(data);
    if (tempLength < 0) return -1;
    quint16 dpListLength = (quint16)(tempLength & 0xFFFF);
    if (size < dpListLength) return -1;

    size -= dpListLength;

    // Check description length
    tempLength = qefi_loadopt_description_length(data);
    if (tempLength < 0) return -1;
    size -= tempLength;
    size -= 2;  // Assume 0x00 0x00 after

    // The remainder is the optional data size
    return size;
}

bool qefi_loadopt_is_valid(const QByteArray &data)
{
    int size = data.size();

    // Check optional data length, it will check:
    //  - the header
    //  - the device path list
    //  - the description
    int optionalSize = qefi_loadopt_optional_data_length(data);
    if (optionalSize < 0) return false;

    // TODO: Check device path

    return true;
}

bool QEFILoadOption::isValidated() const
{
    return m_isValidated;
}

QString QEFILoadOption::name() const
{
    return m_name;
}

bool QEFILoadOption::isVisible() const
{
    return m_isVisible;
}

QString QEFILoadOption::path() const
{
    return m_shortPath;
}

QList<QSharedPointer<QEFIDevicePath> > QEFILoadOption::devicePathList() const
{
    return m_devicePathList;
}

QEFILoadOption::QEFILoadOption(QByteArray &bootData)
    : m_isValidated(false)
{
    parse(bootData);
}

bool QEFILoadOption::parse(QByteArray &bootData)
{
    m_isValidated = false;
    if (qefi_loadopt_is_valid(bootData)) {
        struct qefi_load_option_header *header =
            (struct qefi_load_option_header *)bootData.data();
        m_isVisible = (qFromLittleEndian<quint32>(header->attributes)
            & QEFI_LOAD_OPTION_ACTIVE);
        m_name = qefi_extract_name(bootData);
        m_shortPath = qefi_extract_path(bootData);

        m_isValidated = true;

        // Parse the device path if exists
        int dp_list_length = qefi_loadopt_dp_list_length(bootData);
        if (dp_list_length >= 0) {
            int dp_infered_length = bootData.size() -     // Optional + DP
                sizeof(struct qefi_load_option_header) -    // Header
                (m_name.length() + 1) * 2;                  // Description

            // Get the device list count
            struct qefi_device_path_header *dp_header_pointer =
                (struct qefi_device_path_header *)(((quint8 *)header) +
                    sizeof(struct qefi_load_option_header) +
                    (m_name.length() + 1) * 2);
            int dp_list_count = qefi_dp_count(dp_header_pointer,
                dp_list_length < dp_infered_length ?
                dp_list_length : dp_infered_length);
            for (int i = 0; i < dp_list_count; i++) {
                int tempLength = qefi_dp_length(dp_header_pointer);
                if (tempLength < 0) break;

                // TODO: Parse DP
                qDebug() << "Parsing a device path" << i + 1 << "length" << tempLength;
                QEFIDevicePath *path = qefi_parse_dp(dp_header_pointer, tempLength);
                if (path != nullptr) {
                    m_devicePathList.append(QSharedPointer<QEFIDevicePath>(path));
                }

                dp_header_pointer = (struct qefi_device_path_header *)
                    (((quint8 *)dp_header_pointer) + tempLength);
            }
        }
    }
    return m_isValidated;
}

QEFILoadOption::~QEFILoadOption()
{
    m_devicePathList.clear();
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

// Subclasses for media
QEFIDevicePathMediaHD::QEFIDevicePathMediaHD(quint32 partitionNumber,
    quint64 start, quint64 size, quint8 *signature,
    quint8 format, quint8 signatureType)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_HD),
    m_partitionNumber(partitionNumber), m_start(start), m_size(size),
    m_format(format), m_signatureType(signatureType), m_gptGuid()
{
    for (int i = 0; i < 16; i++) {
        m_signature[i] = signature[i];
    }

    m_mbrSignature = 0;
    switch (m_signatureType) {
        case QEFIDevicePathMediaHDSignatureType::GUID:
            m_gptGuid = qefi_format_guid(m_signature);
        break;
        case QEFIDevicePathMediaHDSignatureType::MBR:
            m_mbrSignature = (quint32)m_signature[0] |
                (quint32)m_signature[1] << 8 |
                (quint32)m_signature[2] << 16 |
                (quint32)m_signature[3] << 24;
        break;
    }
}

QEFIDevicePathMediaCDROM::QEFIDevicePathMediaCDROM(quint32 entry,
    quint64 partitionRba, quint64 sectors)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_CDROM),
    m_bootCatalogEntry(entry), m_partitionRba(partitionRba),
    m_sectors(sectors) {}

QEFIDevicePathMediaVendor::QEFIDevicePathMediaVendor(QUuid vendorGuid,
    QByteArray vendorData)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_Vendor),
    m_vendorGuid(vendorGuid), m_vendorData(vendorData) {}

QEFIDevicePathMediaFile::QEFIDevicePathMediaFile(QString name)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_File),
    m_name(name) {}

QString QEFIDevicePathMediaFile::name() const { return m_name; }

QEFIDevicePathMediaProtocol::QEFIDevicePathMediaProtocol(QUuid protocolGuid)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_Protocol),
    m_protocolGuid(protocolGuid) {}

QEFIDevicePathMediaFirmwareFile::QEFIDevicePathMediaFirmwareFile(QByteArray piInfo)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_FirmwareFile),
    m_piInfo(piInfo) {}

QEFIDevicePathMediaFirmwareVolume::QEFIDevicePathMediaFirmwareVolume(QByteArray piInfo)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume),
    m_piInfo(piInfo) {}

QEFIDevicePathMediaRelativeOffset::QEFIDevicePathMediaRelativeOffset(quint32 reserved,
    quint64 firstByte, quint64 lastByte)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_RelativeOffset),
    m_reserved(reserved), m_firstByte(firstByte), m_lastByte(lastByte) {}

QEFIDevicePathMediaRAMDisk::QEFIDevicePathMediaRAMDisk(
    quint64 startAddress, quint64 endAddress,
    QUuid disktTypeGuid, quint16 instanceNumber)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_RamDisk),
    m_startAddress(startAddress), m_endAddress(endAddress),
    m_disktTypeGuid(disktTypeGuid), m_instanceNumber(instanceNumber) {}

// Subclasses for message
quint8 QEFIDevicePathMessageATAPI::primary() const
{
    return m_primary;
}

quint8 QEFIDevicePathMessageATAPI::slave() const
{
    return m_slave;
}

quint16 QEFIDevicePathMessageATAPI::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageATAPI::QEFIDevicePathMessageATAPI(
        quint8 primary, quint8 slave, quint16 lun)
    : QEFIDevicePathMessage(MSG_ATAPI),
    m_primary(primary), m_slave(slave), m_lun(lun) {}

quint16 QEFIDevicePathMessageSCSI::target() const
{
    return m_target;
}

quint16 QEFIDevicePathMessageSCSI::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageSCSI::QEFIDevicePathMessageSCSI(
        quint16 target, quint16 lun)
    : QEFIDevicePathMessage(MSG_SCSI),
    m_target(target), m_lun(lun) {}

quint32 QEFIDevicePathMessageFibreChan::reserved() const
{
    return m_reserved;
}

quint64 QEFIDevicePathMessageFibreChan::wwn() const
{
    return m_wwn;
}

quint64 QEFIDevicePathMessageFibreChan::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageFibreChan::QEFIDevicePathMessageFibreChan(
        quint32 reserved, quint64 wwn, quint64 lun)
    : QEFIDevicePathMessage(MSG_FibreChan),
    m_reserved(reserved), m_wwn(wwn), m_lun(lun) {}

quint32 QEFIDevicePathMessage1394::reserved() const
{
    return m_reserved;
}

quint64 QEFIDevicePathMessage1394::guid() const
{
    return m_guid;
}

QEFIDevicePathMessage1394::QEFIDevicePathMessage1394(
        quint32 reserved, quint64 guid)
    : QEFIDevicePathMessage(MSG_1394),
    m_reserved(reserved), m_guid(guid) {}

quint8 QEFIDevicePathMessageUSB::parentPort() const
{
    return m_parentPort;
}

quint8 QEFIDevicePathMessageUSB::usbInterface() const
{
    return m_interface;
}

QEFIDevicePathMessageUSB::QEFIDevicePathMessageUSB(
        quint8 parentPort, quint8 inter)
    : QEFIDevicePathMessage(MSG_USB),
    m_parentPort(parentPort), m_interface(inter) {}

quint32 QEFIDevicePathMessageI2O::target() const
{
    return m_target;
}

QEFIDevicePathMessageI2O::QEFIDevicePathMessageI2O(quint32 target)
    : QEFIDevicePathMessage(MSG_I2O), m_target(target) {}

// TODO: Add MSG_InfiniBand  = 0x09

QUuid QEFIDevicePathMessageVendor::vendorGuid() const
{
    return m_vendorGuid;
}

QByteArray QEFIDevicePathMessageVendor::vendorData() const
{
    return m_vendorData;
}

QEFIDevicePathMessageVendor::QEFIDevicePathMessageVendor(
        QUuid vendorGuid, QByteArray vendorData)
    : QEFIDevicePathMessage(MSG_Vendor),
    m_vendorGuid(vendorGuid), m_vendorData(vendorData) {}

quint8 QEFIDevicePathMessageMACAddr::interfaceType() const
{
    return m_interfaceType;
}

QEFIDevicePathMessageMACAddr::QEFIDevicePathMessageMACAddr(
        quint8 *macAddress, quint8 interfaceType)
    : QEFIDevicePathMessage(MSG_MACAddr),
    m_interfaceType(interfaceType)
{
    for (int i = 0; i < 32; i++) {
        m_macAddress[i] = macAddress[i];
    }
}

quint16 QEFIDevicePathMessageIPv4Addr::localPort() const
{
    return m_localPort;
}

quint16 QEFIDevicePathMessageIPv4Addr::remotePort() const
{
    return m_remotePort;
}

quint16 QEFIDevicePathMessageIPv4Addr::protocol() const
{
    return m_protocol;
}

quint8 QEFIDevicePathMessageIPv4Addr::staticIPAddress() const
{
    return m_staticIPAddress;
}

QEFIDevicePathMessageIPv4Addr::QEFIDevicePathMessageIPv4Addr(
        quint8 *localIPv4Addr, quint8 *remoteIPv4Addr,
    quint16 localPort, quint16 remotePort, quint16 protocol,
    quint8 staticIPAddr, quint8 *gateway, quint8 *netmask)
    : QEFIDevicePathMessage(MSG_IPv4),
    m_localPort(localPort), m_remotePort(remotePort),
    m_protocol(protocol), m_staticIPAddress(staticIPAddr)
{
    for (int i = 0; i < 4; i++) {
        m_localIPv4Address[i] = localIPv4Addr[i];
        m_remoteIPv4Address[i] = remoteIPv4Addr[i];
        m_gateway[i] = gateway[i];
        m_netmask[i] = netmask[i];
    }
}

quint16 QEFIDevicePathMessageIPv6Addr::localPort() const
{
    return m_localPort;
}

quint16 QEFIDevicePathMessageIPv6Addr::remotePort() const
{
    return m_remotePort;
}

quint16 QEFIDevicePathMessageIPv6Addr::protocol() const
{
    return m_protocol;
}

quint8 QEFIDevicePathMessageIPv6Addr::ipAddressOrigin() const
{
    return m_ipAddressOrigin;
}

quint8 QEFIDevicePathMessageIPv6Addr::prefixLength() const
{
    return m_prefixLength;
}

quint8 QEFIDevicePathMessageIPv6Addr::gatewayIPv6Address() const
{
    return m_gatewayIPv6Address;
}

QEFIDevicePathMessageIPv6Addr::QEFIDevicePathMessageIPv6Addr(
        quint8 *localIPv6Addr, quint8 *remoteIPv6Addr,
    quint16 localPort, quint16 remotePort, quint16 protocol,
    quint8 ipAddrOrigin, quint8 prefixLength, quint8 gatewayIPv6Addr)
    : QEFIDevicePathMessage(MSG_IPv6),
    m_localPort(localPort), m_remotePort(remotePort),
    m_protocol(protocol), m_ipAddressOrigin(ipAddrOrigin),
    m_prefixLength(prefixLength),
    m_gatewayIPv6Address(gatewayIPv6Addr)
{
    for (int i = 0; i < 16; i++) {
        m_localIPv6Address[i] = localIPv6Addr[i];
        m_remoteIPv6Address[i] = remoteIPv6Addr[i];
    }
}

quint32 QEFIDevicePathMessageUART::reserved() const
{
    return m_reserved;
}

quint64 QEFIDevicePathMessageUART::baudRate() const
{
    return m_baudRate;
}

quint8 QEFIDevicePathMessageUART::dataBits() const
{
    return m_dataBits;
}

quint8 QEFIDevicePathMessageUART::parity() const
{
    return m_parity;
}

quint8 QEFIDevicePathMessageUART::stopBits() const
{
    return m_stopBits;
}

QEFIDevicePathMessageUART::QEFIDevicePathMessageUART(
        quint32 reserved, quint64 baudRate, quint8 dataBits,
    quint8 parity, quint8 stopBits)
    : QEFIDevicePathMessage(MSG_UART),
    m_reserved(reserved), m_baudRate(baudRate),
    m_dataBits(dataBits), m_parity(parity),
    m_stopBits(stopBits) {}

quint16 QEFIDevicePathMessageUSBClass::vendorId() const
{
    return m_vendorId;
}

quint16 QEFIDevicePathMessageUSBClass::productId() const
{
    return m_productId;
}

quint8 QEFIDevicePathMessageUSBClass::deviceClass() const
{
    return m_deviceClass;
}

quint8 QEFIDevicePathMessageUSBClass::deviceSubclass() const
{
    return m_deviceSubclass;
}

quint8 QEFIDevicePathMessageUSBClass::deviceProtocol() const
{
    return m_deviceProtocol;
}

QEFIDevicePathMessageUSBClass::QEFIDevicePathMessageUSBClass(
        quint16 vendorId, quint16 productId, quint8 deviceClass,
    quint8 deviceSubclass, quint8 deviceProtocol)
    : QEFIDevicePathMessage(MSG_USBClass),
    m_vendorId(vendorId), m_productId(productId),
    m_deviceClass(deviceClass), m_deviceSubclass(deviceSubclass),
    m_deviceProtocol(deviceProtocol) {}

quint16 QEFIDevicePathMessageUSBWWID::vendorId() const
{
    return m_vendorId;
}

quint16 QEFIDevicePathMessageUSBWWID::productId() const
{
    return m_productId;
}

QEFIDevicePathMessageUSBWWID::QEFIDevicePathMessageUSBWWID(
        quint16 vendorId, quint16 productId, quint16 *sn)
    : QEFIDevicePathMessage(MSG_USBWWID),
    m_vendorId(vendorId), m_productId(productId)
{
    // TODO: Clarify the SN length
}

quint8 QEFIDevicePathMessageLUN::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageLUN::QEFIDevicePathMessageLUN(quint8 lun)
    : QEFIDevicePathMessage(MSG_LUN), m_lun(lun) {}

quint16 QEFIDevicePathMessageSATA::hbaPort() const
{
    return m_hbaPort;
}

quint16 QEFIDevicePathMessageSATA::portMultiplierPort() const
{
    return m_portMultiplierPort;
}

quint16 QEFIDevicePathMessageSATA::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageSATA::QEFIDevicePathMessageSATA(
        quint16 hbaPort, quint16 portMultiplierPort, quint8 lun)
    : QEFIDevicePathMessage(MSG_SATA), m_hbaPort(hbaPort),
    m_portMultiplierPort(portMultiplierPort), m_lun(lun) {}

quint16 QEFIDevicePathMessageISCSI::protocol() const
{
    return m_protocol;
}

quint16 QEFIDevicePathMessageISCSI::options() const
{
    return m_options;
}

quint16 QEFIDevicePathMessageISCSI::tpgt() const
{
    return m_tpgt;
}

QString QEFIDevicePathMessageISCSI::targetName() const
{
    return m_targetName;
}

QEFIDevicePathMessageISCSI::QEFIDevicePathMessageISCSI(
        quint16 protocol, quint16 options, quint8 *lun,
    quint16 tpgt, QString targetName)
    : QEFIDevicePathMessage(MSG_ISCSI),
    m_protocol(protocol), m_options(options),
    m_tpgt(tpgt), m_targetName(targetName)
{
    for (int i = 0; i < 8; i++) {
        m_lun[i] = lun[i];
    }
};

quint16 QEFIDevicePathMessageVLAN::vlanID() const
{
    return m_vlanID;
}

QEFIDevicePathMessageVLAN::QEFIDevicePathMessageVLAN(quint16 vlanID)
    : QEFIDevicePathMessage(MSG_VLAN), m_vlanID(vlanID) {}

QEFIDevicePathMessageFibreChanEx::QEFIDevicePathMessageFibreChanEx(
    quint32 reserved, quint8 *wwn, quint8 *lun)
    : QEFIDevicePathMessage(MSG_FibreChanEx), m_reserved(reserved)
{
    for (int i = 0; i < 8; i++) {
        m_wwn[i] = wwn[i];
        m_lun[i] = lun[i];
    }
}

quint8 QEFIDevicePathMessageSASEx::deviceTopologyInfo() const
{
    return m_deviceTopologyInfo;
}

quint8 QEFIDevicePathMessageSASEx::driveBayID() const
{
    return m_driveBayID;
}

quint16 QEFIDevicePathMessageSASEx::rtp() const
{
    return m_rtp;
}

QEFIDevicePathMessageSASEx::QEFIDevicePathMessageSASEx(
        quint8 *sasAddress, quint8 *lun, quint8 deviceTopologyInfo,
    quint8 driveBayID, quint16 rtp)
    : QEFIDevicePathMessage(MSG_SASEX),
    m_deviceTopologyInfo(deviceTopologyInfo),
    m_driveBayID(driveBayID), m_rtp(rtp)
{
    for (int i = 0; i < 8; i++) {
        m_sasAddress[i] = sasAddress[i];
        m_lun[i] = lun[i];
    }
}

quint32 QEFIDevicePathMessageNVME::namespaceID() const
{
    return m_namespaceID;
}

QEFIDevicePathMessageNVME::QEFIDevicePathMessageNVME(
        quint32 nid, quint8 *ieeeEui64)
    : QEFIDevicePathMessage(MSG_NVME), m_namespaceID(nid)
{
    memcpy(m_ieeeEui64, ieeeEui64, sizeof(quint8) * 64);
}

QUrl QEFIDevicePathMessageURI::uri() const
{
    return m_uri;
}

QEFIDevicePathMessageURI::QEFIDevicePathMessageURI(QUrl uri)
    : QEFIDevicePathMessage(MSG_URI), m_uri(uri) {}

quint8 QEFIDevicePathMessageUFS::targetID() const
{
    return m_targetID;
}

quint8 QEFIDevicePathMessageUFS::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageUFS::QEFIDevicePathMessageUFS(
        quint8 targetID, quint8 lun)
    : QEFIDevicePathMessage(MSG_UFS),
    m_targetID(targetID), m_lun(lun) {}

quint8 QEFIDevicePathMessageSD::slotNumber() const
{
    return m_slotNumber;
}

QEFIDevicePathMessageSD::QEFIDevicePathMessageSD(quint8 slot)
    : QEFIDevicePathMessage(MSG_SD), m_slotNumber(slot) {}

QEFIDevicePathMessageBT::QEFIDevicePathMessageBT(quint8 *address)
    : QEFIDevicePathMessage(MSG_BT)
{
    for (int i = 0; i < 6; i++)
        m_address[i] = address[i];
}

QString QEFIDevicePathMessageWiFi::ssid() const
{
    return m_ssid;
}

QEFIDevicePathMessageWiFi::QEFIDevicePathMessageWiFi(QString ssid)
    : QEFIDevicePathMessage(MSG_WiFi), m_ssid(ssid) {}

quint8 QEFIDevicePathMessageEMMC::slotNumber() const
{
    return m_slotNumber;
}

QEFIDevicePathMessageEMMC::QEFIDevicePathMessageEMMC(quint8 slot)
    : QEFIDevicePathMessage(MSG_EMMC), m_slotNumber(slot) {}

quint8 QEFIDevicePathMessageBTLE::addressType() const
{
    return m_addressType;
}

QEFIDevicePathMessageBTLE::QEFIDevicePathMessageBTLE(
        quint8 *address, quint8 addressType)
    : QEFIDevicePathMessage(MSG_BTLE),
    m_addressType(addressType)
{
    for (int i = 0; i < 6; i++)
        m_address[i] = address[i];
}

QEFIDevicePathMessageDNS::QEFIDevicePathMessageDNS()
    : QEFIDevicePathMessage(MSG_DNS) {}

QUuid QEFIDevicePathMessageNVDIMM::uuid() const
{
    return m_uuid;
}

QEFIDevicePathMessageNVDIMM::QEFIDevicePathMessageNVDIMM(QUuid uuid)
    : QEFIDevicePathMessage(MSG_NVDIMM), m_uuid(uuid) {}

QUuid qefi_format_guid(const quint8 *data)
{
    return QUuid(
        (uint)((uint)data[0] | (uint)data[1] << 8 |
            (uint)data[2] << 16 | (uint)data[3] << 24), // LE
        (ushort)data[4] | (ushort)data[5] << 8, // LE
        (ushort)data[6] | (ushort)data[7] << 8, // LE
        (uchar)data[8], (uchar)data[9],         // BE
        (uchar)data[10], (uchar)data[11],
        (uchar)data[12], (uchar)data[13],
        (uchar)data[14], (uchar)data[15]);
}
