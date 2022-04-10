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
