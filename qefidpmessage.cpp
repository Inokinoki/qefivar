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

// Message parsing
QEFIDevicePath *qefi_parse_dp_message_atapi(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_ATAPI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) + sizeof(quint8) + sizeof(quint16))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SCSI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint16) + sizeof(quint16))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_FibreChan)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32) + sizeof(quint64) + sizeof(quint64))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_1394)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32) + sizeof(quint64))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_USB)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) + sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 parentPort = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 inter = *dp_inner_pointer;
    return new QEFIDevicePathMessageUSB(parentPort, inter);
}

QEFIDevicePath *qefi_parse_dp_message_i2o(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_I2O)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 target =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    return new QEFIDevicePathMessageI2O(target);
}

QEFIDevicePath *qefi_parse_dp_message_infiniband(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_InfiniBand)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32) + sizeof(quint64) + sizeof(quint64) +
        sizeof(quint64) + sizeof(quint64) + sizeof(quint64))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 resourceFlags =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 portGID1 =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 portGID2 =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 sharedField =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 targetPortID =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 deviceID =
        qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathMessageInfiniBand(resourceFlags,
        portGID1, portGID2, sharedField,
        targetPortID, deviceID);
}

QEFIDevicePath *qefi_parse_dp_message_vendor(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_Vendor)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) * 16)
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_MACAddr)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) * 32 + sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 *macAddress = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 32;
    quint8 interfaceType = *dp_inner_pointer;
    return new QEFIDevicePathMessageMACAddr(macAddress, interfaceType);
}

QEFIDevicePath *qefi_parse_dp_message_ipv4(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_IPv4)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) * 4 + sizeof(quint8) * 4 +
        sizeof(quint16) + sizeof(quint16) +
        sizeof(quint16) + sizeof(quint8) +
        sizeof(quint8) * 4 + sizeof(quint8) * 4)
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_IPv6)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) * 16 + sizeof(quint8) * 16 +
        sizeof(quint16) + sizeof(quint16) + sizeof(quint16) +
        sizeof(quint8) + sizeof(quint8) + sizeof(quint8))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_UART)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32) + sizeof(quint64) +
        sizeof(quint8) + sizeof(quint8) + sizeof(quint8))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_USBClass)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint16) + sizeof(quint16) +
        sizeof(quint8) + sizeof(quint8) + sizeof(quint8))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_USBWWID)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint16) + sizeof(quint16))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_LUN)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 lun = *dp_inner_pointer;
    return new QEFIDevicePathMessageLUN(lun);
}

QEFIDevicePath *qefi_parse_dp_message_sata(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SATA)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint16) + sizeof(quint16) + sizeof(quint8))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_ISCSI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint16) + sizeof(quint16) +
        sizeof(quint8) * 16 + sizeof(quint16))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_VLAN)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint16))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint16 vlanID =
        qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    return new QEFIDevicePathMessageVLAN(vlanID);
}

QEFIDevicePath *qefi_parse_dp_message_fibre_chan_ex(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_FibreChanEx)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32) + sizeof(quint8) * 8 +
        sizeof(quint8) * 8)
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SASEX)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint16) + sizeof(quint8) * 8 +
        sizeof(quint8) * 8 + sizeof(quint8) +
        sizeof(quint8) + sizeof(quint16))
        return nullptr;

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
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_NVME)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint32))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 nid =
        qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    return new QEFIDevicePathMessageNVME(nid, dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_uri(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_URI)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUrl uri(QString(QByteArray((const char *)dp_inner_pointer,
        length - (dp_inner_pointer - (quint8 *)dp))));
    return new QEFIDevicePathMessageURI(uri);
}

QEFIDevicePath *qefi_parse_dp_message_ufs(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_UFS)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) + sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 targetID = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 lun = *dp_inner_pointer;
    return new QEFIDevicePathMessageUFS(targetID, lun);
}

QEFIDevicePath *qefi_parse_dp_message_sd(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_SD)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMessageSD(*dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_bt(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_BT)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) * 6)
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMessageBT(dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_wifi(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_WiFi)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    int ssid_len = length - sizeof(struct qefi_device_path_header);
    QString ssid(QByteArray((const char *)dp_inner_pointer,
        ssid_len < 32 ? ssid_len : 32));
    return new QEFIDevicePathMessageWiFi(ssid);
}

QEFIDevicePath *qefi_parse_dp_message_emmc(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_EMMC)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMessageEMMC(*dp_inner_pointer);
}

QEFIDevicePath *qefi_parse_dp_message_btle(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_BTLE)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) * 6 + sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 *address = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 6;
    quint8 addressType = *dp_inner_pointer;
    return new QEFIDevicePathMessageBTLE(address, addressType);
}

QEFIDevicePath *qefi_parse_dp_message_dns(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_DNS)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint8 is_ipv6 = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    // TODO: Parse addresses
    return new QEFIDevicePathMessageDNS();
}

QEFIDevicePath *qefi_parse_dp_message_nvdimm(
    struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE) return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Message ||
        dp->subtype != QEFIDevicePathMessageSubType::MSG_NVDIMM)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0) return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE +
        sizeof(quint8) * 8)
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUuid uuid = qefi_format_guid(dp_inner_pointer);
    return new QEFIDevicePathMessageNVDIMM(uuid);
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
            qDebug() << "Parsing DP message InfiniBand";
            return qefi_parse_dp_message_infiniband(dp, length);
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


// Message formating
QByteArray qefi_format_dp_message_atapi(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_ATAPI)
        return QByteArray();
    QEFIDevicePathMessageATAPI *dp_instance =
        dynamic_cast<QEFIDevicePathMessageATAPI *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->primary());
    buffer.append(dp_instance->slave());
    quint16 lun =
        qToLittleEndian<quint16>(dp_instance->lun());
    buffer.append((const char *)&lun, sizeof(quint16));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_scsi(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_SCSI)
        return QByteArray();
    QEFIDevicePathMessageSCSI *dp_instance =
        dynamic_cast<QEFIDevicePathMessageSCSI *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint16 target =
        qToLittleEndian<quint16>(dp_instance->target());
    buffer.append((const char *)&target, sizeof(quint16));
    quint16 lun =
        qToLittleEndian<quint16>(dp_instance->lun());
    buffer.append((const char *)&lun, sizeof(quint16));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_fibre_chan(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_FibreChan)
        return QByteArray();
    QEFIDevicePathMessageFibreChan *dp_instance =
        dynamic_cast<QEFIDevicePathMessageFibreChan *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 reserved =
        qToLittleEndian<quint32>(dp_instance->reserved());
    buffer.append((const char *)&reserved, sizeof(quint32));
    quint64 wwn =
        qToLittleEndian<quint64>(dp_instance->wwn());
    buffer.append((const char *)&wwn, sizeof(quint64));
    quint64 lun =
        qToLittleEndian<quint64>(dp_instance->lun());
    buffer.append((const char *)&lun, sizeof(quint64));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_1394(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_1394)
        return QByteArray();
    QEFIDevicePathMessage1394 *dp_instance =
        dynamic_cast<QEFIDevicePathMessage1394 *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 reserved =
        qToLittleEndian<quint32>(dp_instance->reserved());
    buffer.append((const char *)&reserved, sizeof(quint32));
    quint64 guid =
        qToLittleEndian<quint64>(dp_instance->guid());
    buffer.append((const char *)&guid, sizeof(quint64));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_usb(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_USB)
        return QByteArray();
    QEFIDevicePathMessageUSB *dp_instance =
        dynamic_cast<QEFIDevicePathMessageUSB *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->parentPort());
    buffer.append(dp_instance->usbInterface());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_i2o(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_I2O)
        return QByteArray();
    QEFIDevicePathMessageI2O *dp_instance =
        dynamic_cast<QEFIDevicePathMessageI2O *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 target =
        qToLittleEndian<quint32>(dp_instance->target());
    buffer.append((const char *)&target, sizeof(quint32));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_infiniband(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_InfiniBand)
        return QByteArray();
    QEFIDevicePathMessageInfiniBand *dp_instance =
        dynamic_cast<QEFIDevicePathMessageInfiniBand *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 resourceFlags =
        qToLittleEndian<quint32>(dp_instance->resourceFlags());
    buffer.append((const char *)&resourceFlags, sizeof(quint32));
    quint64 portGID1 =
        qToLittleEndian<quint64>(dp_instance->portGID1());
    buffer.append((const char *)&portGID1, sizeof(quint64));
    quint64 portGID2 =
        qToLittleEndian<quint64>(dp_instance->portGID2());
    buffer.append((const char *)&portGID2, sizeof(quint64));
    quint64 sharedField =
        qToLittleEndian<quint64>(dp_instance->serviceID());
    buffer.append((const char *)&sharedField, sizeof(quint64));
    quint64 targetPortID =
        qToLittleEndian<quint64>(dp_instance->targetPortID());
    buffer.append((const char *)&targetPortID, sizeof(quint64));
    quint64 deviceID =
        qToLittleEndian<quint64>(dp_instance->deviceID());
    buffer.append((const char *)&deviceID, sizeof(quint64));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_vendor(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_Vendor)
        return QByteArray();
    QEFIDevicePathMessageVendor *dp_instance =
        dynamic_cast<QEFIDevicePathMessageVendor *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(qefi_rfc4122_to_guid(dp_instance->vendorGuid().toRfc4122()));
    buffer.append(dp_instance->vendorData());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_mac_addr(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_MACAddr)
        return QByteArray();
    QEFIDevicePathMessageMACAddr *dp_instance =
        dynamic_cast<QEFIDevicePathMessageMACAddr *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    QEFIDevicePathMessageMACAddress address =
        dp_instance->macAddress();
    for (int i = 0; i < 32; i++) {
        // Append Mac Addr
        buffer.append((const char)address.address[i]);
    }
    buffer.append(dp_instance->interfaceType());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_ipv4(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_IPv4)
        return QByteArray();
    QEFIDevicePathMessageIPv4Addr *dp_instance =
        dynamic_cast<QEFIDevicePathMessageIPv4Addr *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    QEFIIPv4Address address = dp_instance->localIPv4Address();
    for (int i = 0; i < 4; i++) {
        // Append localIPv4Address
        buffer.append((const char)address.address[i]);
    }
    address = dp_instance->remoteIPv4Address();
    for (int i = 0; i < 4; i++) {
        // Append remoteIPv4Address
        buffer.append((const char)address.address[i]);
    }
    quint16 localPort =
        qToLittleEndian<quint16>(dp_instance->localPort());
    buffer.append((const char *)&localPort, sizeof(quint16));
    quint16 remotePort =
        qToLittleEndian<quint16>(dp_instance->remotePort());
    buffer.append((const char *)&remotePort, sizeof(quint16));
    quint16 protocol =
        qToLittleEndian<quint16>(dp_instance->protocol());
    buffer.append((const char *)&protocol, sizeof(quint16));
    buffer.append(dp_instance->staticIPAddress());
    address = dp_instance->gateway();
    for (int i = 0; i < 4; i++) {
        // Append gateway
        buffer.append((const char)address.address[i]);
    }
    address = dp_instance->netmask();
    for (int i = 0; i < 4; i++) {
        // Append netmask
        buffer.append((const char)address.address[i]);
    }

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_ipv6(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_IPv6)
        return QByteArray();
    QEFIDevicePathMessageIPv6Addr *dp_instance =
        dynamic_cast<QEFIDevicePathMessageIPv6Addr *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    QEFIIPv6Address address = dp_instance->localIPv6Address();
    for (int i = 0; i < 16; i++) {
        // Append localIPv6Address
        buffer.append((const char)address.address[i]);
    }
    address = dp_instance->remoteIPv6Address();
    for (int i = 0; i < 16; i++) {
        // Append remoteIPv6Address
        buffer.append((const char)address.address[i]);
    }
    quint16 localPort =
        qToLittleEndian<quint16>(dp_instance->localPort());
    buffer.append((const char *)&localPort, sizeof(quint16));
    quint16 remotePort =
        qToLittleEndian<quint16>(dp_instance->remotePort());
    buffer.append((const char *)&remotePort, sizeof(quint16));
    quint16 protocol =
        qToLittleEndian<quint16>(dp_instance->protocol());
    buffer.append((const char *)&protocol, sizeof(quint16));
    buffer.append(dp_instance->ipAddressOrigin());
    buffer.append(dp_instance->prefixLength());
    buffer.append(dp_instance->gatewayIPv6Address());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_uart(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_UART)
        return QByteArray();
    QEFIDevicePathMessageUART *dp_instance =
        dynamic_cast<QEFIDevicePathMessageUART *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 reserved =
        qToLittleEndian<quint32>(dp_instance->reserved());
    buffer.append((const char *)&reserved, sizeof(quint32));
    quint64 baudRate =
        qToLittleEndian<quint64>(dp_instance->baudRate());
    buffer.append((const char *)&baudRate, sizeof(quint64));
    buffer.append(dp_instance->dataBits());
    buffer.append(dp_instance->parity());
    buffer.append(dp_instance->stopBits());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_usb_class(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_USBClass)
        return QByteArray();
    QEFIDevicePathMessageUSBClass *dp_instance =
        dynamic_cast<QEFIDevicePathMessageUSBClass *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint16 vendorId =
        qToLittleEndian<quint16>(dp_instance->vendorId());
    buffer.append((const char *)&vendorId, sizeof(quint16));
    quint16 productId =
        qToLittleEndian<quint16>(dp_instance->productId());
    buffer.append((const char *)&productId, sizeof(quint16));
    buffer.append(dp_instance->deviceClass());
    buffer.append(dp_instance->deviceSubclass());
    buffer.append(dp_instance->deviceProtocol());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_usb_wwid(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_USBWWID)
        return QByteArray();
    QEFIDevicePathMessageUSBWWID *dp_instance =
        dynamic_cast<QEFIDevicePathMessageUSBWWID *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint16 vendorId =
        qToLittleEndian<quint16>(dp_instance->vendorId());
    buffer.append((const char *)&vendorId, sizeof(quint16));
    quint16 productId =
        qToLittleEndian<quint16>(dp_instance->productId());
    buffer.append((const char *)&productId, sizeof(quint16));
    // TODO: Append SN

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_lun(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_LUN)
        return QByteArray();
    QEFIDevicePathMessageLUN *dp_instance =
        dynamic_cast<QEFIDevicePathMessageLUN *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->lun());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_sata(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_SATA)
        return QByteArray();
    QEFIDevicePathMessageSATA *dp_instance =
        dynamic_cast<QEFIDevicePathMessageSATA *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint16 hbaPort =
        qToLittleEndian<quint16>(dp_instance->hbaPort());
    buffer.append((const char *)&hbaPort, sizeof(quint16));
    quint16 portMultiplierPort =
        qToLittleEndian<quint16>(dp_instance->portMultiplierPort());
    buffer.append((const char *)&portMultiplierPort, sizeof(quint16));
    quint16 lun =
        qToLittleEndian<quint16>(dp_instance->lun());
    buffer.append((const char *)&lun, sizeof(quint16));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_iscsi(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_ISCSI)
        return QByteArray();
    QEFIDevicePathMessageISCSI *dp_instance =
        dynamic_cast<QEFIDevicePathMessageISCSI *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint16 protocol =
        qToLittleEndian<quint16>(dp_instance->protocol());
    buffer.append((const char *)&protocol, sizeof(quint16));
    quint16 options =
        qToLittleEndian<quint16>(dp_instance->options());
    buffer.append((const char *)&options, sizeof(quint16));
    QEFIDevicePathMessageLun lun = dp_instance->lun();
    for (int i = 0; i < 8; i++) {
        // Append lun
        buffer.append((const char)lun.data[i]);
    }
    quint16 tpgt =
        qToLittleEndian<quint16>(dp_instance->tpgt());
    buffer.append((const char *)&tpgt, sizeof(quint16));
    // TODO: Clarify the encoding
    buffer.append(dp_instance->targetName().toUtf8());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_vlan(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_VLAN)
        return QByteArray();
    QEFIDevicePathMessageVLAN *dp_instance =
        dynamic_cast<QEFIDevicePathMessageVLAN *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint16 vlanID =
        qToLittleEndian<quint16>(dp_instance->vlanID());
    buffer.append((const char *)&vlanID, sizeof(quint16));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_fibre_chan_ex(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_FibreChanEx)
        return QByteArray();
    QEFIDevicePathMessageFibreChanEx *dp_instance =
        dynamic_cast<QEFIDevicePathMessageFibreChanEx *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 reserved = 0;
    buffer.append((const char *)&reserved, sizeof(quint32));
    QEFIDevicePathMessageLun wwn = dp_instance->wwn();
    for (int i = 0; i < 8; i++) {
        buffer.append((const char)wwn.data[i]);
    }
    QEFIDevicePathMessageLun lun = dp_instance->lun();
    for (int i = 0; i < 8; i++) {
        buffer.append((const char)lun.data[i]);
    }

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_sas_ex(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_SASEX)
        return QByteArray();
    QEFIDevicePathMessageSASEx *dp_instance =
        dynamic_cast<QEFIDevicePathMessageSASEx *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    QEFIDevicePathMessageSASAddress address =
        dp_instance->sasAddress();
    for (int i = 0; i < 8; i++) {
        // Append sasAddress
        buffer.append((const char)address.address[i]);
    }
    QEFIDevicePathMessageLun lun = dp_instance->lun();
    for (int i = 0; i < 8; i++) {
        // Append lun
        buffer.append((const char)lun.data[i]);
    }
    buffer.append(dp_instance->deviceTopologyInfo());
    buffer.append(dp_instance->driveBayID());
    quint16 rtp =
        qToLittleEndian<quint16>(dp_instance->rtp());
    buffer.append((const char *)&rtp, sizeof(quint16));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_nvme(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_NVME)
        return QByteArray();
    QEFIDevicePathMessageNVME *dp_instance =
        dynamic_cast<QEFIDevicePathMessageNVME *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 namespaceID =
        qToLittleEndian<quint32>(dp_instance->namespaceID());
    buffer.append((const char *)&namespaceID, sizeof(quint32));

    QEFIDevicePathMessageEUI64 eui = dp_instance->ieeeEui64();
    for (int i = 0; i < 8; i++) {
        // Append ieeeEui64
        buffer.append((const char)eui.eui[i]);
    }

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_uri(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_URI)
        return QByteArray();
    QEFIDevicePathMessageURI *dp_instance =
        dynamic_cast<QEFIDevicePathMessageURI *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->uri().url().toUtf8());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_ufs(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_UFS)
        return QByteArray();
    QEFIDevicePathMessageUFS *dp_instance =
        dynamic_cast<QEFIDevicePathMessageUFS *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->targetID());
    buffer.append(dp_instance->lun());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_sd(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_SD)
        return QByteArray();
    QEFIDevicePathMessageSD *dp_instance =
        dynamic_cast<QEFIDevicePathMessageSD *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->slotNumber());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_bt(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_BT)
        return QByteArray();
    QEFIDevicePathMessageBT *dp_instance =
        dynamic_cast<QEFIDevicePathMessageBT *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    QEFIDevicePathMessageBTAddress addr =
        dp_instance->address();
    for (int i = 0; i < 6; i++) {
        // Get the BT addr
        buffer.append((const char)addr.address[i]);
    }

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_wifi(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_WiFi)
        return QByteArray();
    QEFIDevicePathMessageWiFi *dp_instance =
        dynamic_cast<QEFIDevicePathMessageWiFi *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->ssid().toUtf8());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_emmc(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_EMMC)
        return QByteArray();
    QEFIDevicePathMessageEMMC *dp_instance =
        dynamic_cast<QEFIDevicePathMessageEMMC *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->slotNumber());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_btle(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_BTLE)
        return QByteArray();
    QEFIDevicePathMessageBTLE *dp_instance =
        dynamic_cast<QEFIDevicePathMessageBTLE *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    // Append the address
    QEFIDevicePathMessageBTAddress addr =
        dp_instance->address();
    for (int i = 0; i < 8; i++) {
        buffer.append((const char)addr.address[i]);
    }
    buffer.append(dp_instance->addressType());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_dns(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_DNS)
        return QByteArray();
    QEFIDevicePathMessageDNS *dp_instance =
        dynamic_cast<QEFIDevicePathMessageDNS *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    // TODO: Append the DNS information

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_message_nvdimm(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Message ||
        dp->subType() != QEFIDevicePathMessageSubType::MSG_NVDIMM)
        return QByteArray();
    QEFIDevicePathMessageNVDIMM *dp_instance =
        dynamic_cast<QEFIDevicePathMessageNVDIMM *>(dp);
    if (dp_instance == nullptr) return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(qefi_rfc4122_to_guid(dp_instance->uuid().toRfc4122()));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_private_format_message_subtype(QEFIDevicePath *dp)
{
    // Format message
    quint8 type = dp->type(), subtype = dp->subType();
    if (type != QEFIDevicePathType::DP_Message) return QByteArray();

    switch (subtype) {
        case QEFIDevicePathMessageSubType::MSG_ATAPI:
            qDebug() << "Formating DP message ATAPI";
            return qefi_format_dp_message_atapi(dp);
        case QEFIDevicePathMessageSubType::MSG_SCSI:
            qDebug() << "Formating DP message SCSI";
            return qefi_format_dp_message_scsi(dp);
        case QEFIDevicePathMessageSubType::MSG_FibreChan:
            qDebug() << "Formating DP message FibreChan";
            return qefi_format_dp_message_fibre_chan(dp);
        case QEFIDevicePathMessageSubType::MSG_1394:
            qDebug() << "Formating DP message 1394";
            return qefi_format_dp_message_1394(dp);
        case QEFIDevicePathMessageSubType::MSG_USB:
            qDebug() << "Formating DP message USB";
            return qefi_format_dp_message_usb(dp);
        case QEFIDevicePathMessageSubType::MSG_I2O:
            qDebug() << "Formating DP message I2O";
            return qefi_format_dp_message_i2o(dp);
        case QEFIDevicePathMessageSubType::MSG_InfiniBand:
            qDebug() << "Formating DP message InfiniBand";
            return qefi_format_dp_message_infiniband(dp);
        case QEFIDevicePathMessageSubType::MSG_Vendor:
            qDebug() << "Formating DP message Vendor";
            return qefi_format_dp_message_vendor(dp);
        case QEFIDevicePathMessageSubType::MSG_MACAddr:
            qDebug() << "Formating DP message Mac Address";
            return qefi_format_dp_message_mac_addr(dp);
        case QEFIDevicePathMessageSubType::MSG_IPv4:
            qDebug() << "Formating DP message IPv4";
            return qefi_format_dp_message_ipv4(dp);
        case QEFIDevicePathMessageSubType::MSG_IPv6:
            qDebug() << "Formating DP message IPv6";
            return qefi_format_dp_message_ipv6(dp);
        case QEFIDevicePathMessageSubType::MSG_UART:
            qDebug() << "Formating DP message UART";
            return qefi_format_dp_message_uart(dp);
        case QEFIDevicePathMessageSubType::MSG_USBClass:
            qDebug() << "Formating DP message USB Class";
            return qefi_format_dp_message_usb_class(dp);

        case QEFIDevicePathMessageSubType::MSG_USBWWID:
            qDebug() << "Formating DP message USBWWID";
            return qefi_format_dp_message_usb_wwid(dp);

        case QEFIDevicePathMessageSubType::MSG_LUN:
            qDebug() << "Formating DP message LUN";
            return qefi_format_dp_message_lun(dp);
        case QEFIDevicePathMessageSubType::MSG_SATA:
            qDebug() << "Formating DP message SATA";
            return qefi_format_dp_message_sata(dp);
        case QEFIDevicePathMessageSubType::MSG_ISCSI:
            qDebug() << "Formating DP message ISCSI";
            return qefi_format_dp_message_iscsi(dp);
        case QEFIDevicePathMessageSubType::MSG_VLAN:
            qDebug() << "Formating DP message VLAN";
            return qefi_format_dp_message_vlan(dp);

        case QEFIDevicePathMessageSubType::MSG_FibreChanEx:
            qDebug() << "Formating DP message FibreChanEx";
            return qefi_format_dp_message_fibre_chan_ex(dp);
        case QEFIDevicePathMessageSubType::MSG_SASEX:
            qDebug() << "Formating DP message SASEX";
            return qefi_format_dp_message_sas_ex(dp);

        case QEFIDevicePathMessageSubType::MSG_NVME:
            qDebug() << "Formating DP message NVME";
            return qefi_format_dp_message_nvme(dp);
        case QEFIDevicePathMessageSubType::MSG_URI:
            qDebug() << "Formating DP message URI";
            return qefi_format_dp_message_uri(dp);
        case QEFIDevicePathMessageSubType::MSG_UFS:
            qDebug() << "Formating DP message UFS";
            return qefi_format_dp_message_ufs(dp);
        case QEFIDevicePathMessageSubType::MSG_SD:
            qDebug() << "Formating DP message SD";
            return qefi_format_dp_message_sd(dp);
        case QEFIDevicePathMessageSubType::MSG_BT:
            qDebug() << "Formating DP message Bluetooth";
            return qefi_format_dp_message_bt(dp);
        case QEFIDevicePathMessageSubType::MSG_WiFi:
            qDebug() << "Formating DP message WiFi";
            return qefi_format_dp_message_wifi(dp);
        case QEFIDevicePathMessageSubType::MSG_EMMC:
            qDebug() << "Formating DP message EMMC";
            return qefi_format_dp_message_emmc(dp);
        case QEFIDevicePathMessageSubType::MSG_BTLE:
            qDebug() << "Formating DP message BTLE";
            return qefi_format_dp_message_btle(dp);
        case QEFIDevicePathMessageSubType::MSG_DNS:
            qDebug() << "Formating DP message DNS";
            return qefi_format_dp_message_dns(dp);
        case QEFIDevicePathMessageSubType::MSG_NVDIMM:
            qDebug() << "Formating DP message NVDIMM";
            return qefi_format_dp_message_nvdimm(dp);
    }
    return QByteArray();
}


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


quint64 QEFIDevicePathMessageInfiniBand::targetPortID() const
{
    return m_targetPortID;
}

quint64 QEFIDevicePathMessageInfiniBand::deviceID() const
{
    return m_deviceID;
}

quint64 QEFIDevicePathMessageInfiniBand::portGID1() const
{
    return m_portGID1;
}

quint64 QEFIDevicePathMessageInfiniBand::portGID2() const
{
    return m_portGID2;
}

quint32 QEFIDevicePathMessageInfiniBand::resourceFlags() const
{
    return m_resourceFlags;
}

quint64 QEFIDevicePathMessageInfiniBand::iocGuid() const
{
    return m_sharedField;
}

quint64 QEFIDevicePathMessageInfiniBand::serviceID() const
{
    return m_sharedField;
}

QEFIDevicePathMessageInfiniBand::QEFIDevicePathMessageInfiniBand(
    quint32 resourceFlags, quint64 portGID1, quint64 portGID2,
    quint64 sharedField, quint64 targetPortID,
    quint64 deviceID)
    : QEFIDevicePathMessage(MSG_InfiniBand),
    m_resourceFlags(resourceFlags),
    m_portGID1(portGID1), m_portGID2(portGID2),
    m_sharedField(sharedField), m_targetPortID(targetPortID),
    m_deviceID(deviceID) {}

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
        m_macAddress.address[i] = macAddress[i];
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
        m_localIPv4Address.address[i] = localIPv4Addr[i];
        m_remoteIPv4Address.address[i] = remoteIPv4Addr[i];
        m_gateway.address[i] = gateway[i];
        m_netmask.address[i] = netmask[i];
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

QEFIIPv6Address QEFIDevicePathMessageIPv6Addr::remoteIPv6Address() const
{
    return m_remoteIPv6Address;
}

QEFIIPv6Address QEFIDevicePathMessageIPv6Addr::localIPv6Address() const
{
    return m_localIPv6Address;
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
        m_localIPv6Address.address[i] = localIPv6Addr[i];
        m_remoteIPv6Address.address[i] = remoteIPv6Addr[i];
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

QList<quint16> QEFIDevicePathMessageUSBWWID::serialNumber() const
{
    return m_serialNumber;
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
        m_lun.data[i] = lun[i];
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
        m_wwn.data[i] = wwn[i];
        m_lun.data[i] = lun[i];
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
        m_sasAddress.address[i] = sasAddress[i];
        m_lun.data[i] = lun[i];
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
    memcpy(m_ieeeEui64.eui, ieeeEui64, sizeof(quint8) * 64);
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
        m_address.address[i] = address[i];
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
        m_address.address[i] = address[i];
}

QEFIDevicePathMessageDNS::QEFIDevicePathMessageDNS()
    : QEFIDevicePathMessage(MSG_DNS) {}

QUuid QEFIDevicePathMessageNVDIMM::uuid() const
{
    return m_uuid;
}

QEFIDevicePathMessageNVDIMM::QEFIDevicePathMessageNVDIMM(QUuid uuid)
    : QEFIDevicePathMessage(MSG_NVDIMM), m_uuid(uuid) {}


QEFIIPv4Address QEFIDevicePathMessageIPv4Addr::remoteIPv4Address() const
{
    return m_remoteIPv4Address;
}

QEFIIPv4Address QEFIDevicePathMessageIPv4Addr::gateway() const
{
    return m_gateway;
}

QEFIIPv4Address QEFIDevicePathMessageIPv4Addr::netmask() const
{
    return m_netmask;
}

QEFIIPv4Address QEFIDevicePathMessageIPv4Addr::localIPv4Address() const
{
    return m_localIPv4Address;
}



QEFIDevicePathMessageLun QEFIDevicePathMessageISCSI::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageLun QEFIDevicePathMessageFibreChanEx::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageLun QEFIDevicePathMessageFibreChanEx::wwn() const
{
    return m_wwn;
}

QEFIDevicePathMessageLun QEFIDevicePathMessageSASEx::lun() const
{
    return m_lun;
}

QEFIDevicePathMessageSASAddress QEFIDevicePathMessageSASEx::sasAddress() const
{
    return m_sasAddress;
}

QEFIDevicePathMessageEUI64 QEFIDevicePathMessageNVME::ieeeEui64() const
{
    return m_ieeeEui64;
}

QEFIDevicePathMessageBTAddress QEFIDevicePathMessageBT::address() const
{
    return m_address;
}

QEFIDevicePathMessageBTAddress QEFIDevicePathMessageBTLE::address() const
{
    return m_address;
}


