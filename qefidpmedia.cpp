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


// Media formating
QByteArray qefi_format_dp_media_hdd(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_file(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_cdrom(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_vendor(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_protocol(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_firmware_file(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_fv(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_relative_offset(QEFIDevicePath *dp)
{
    return QByteArray();
}

QByteArray qefi_format_dp_media_ramdisk(QEFIDevicePath *dp)
{
    return QByteArray();
}


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
