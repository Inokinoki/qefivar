#include "qefi.h"

#include <QDebug>
#include <QtEndian>

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
int qefi_dp_count(struct qefi_device_path_header *dp_header_pointer, int max_dp_size);
int qefi_dp_total_size(struct qefi_device_path_header *dp_header_pointer, int max_dp_size);
QString qefi_parse_ucs2_string(quint8 *data, int max_size);
QByteArray qefi_format_string_to_ucs2(const QString &str, bool isEnd);

// Media parsing
QEFIDevicePath *qefi_parse_dp_media_file(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_File)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    return new QEFIDevicePathMediaFile(qefi_parse_ucs2_string(dp_inner_pointer, length - sizeof(struct qefi_device_path_header)));
}

QEFIDevicePath *qefi_parse_dp_media_hdd(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_HD)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE + sizeof(quint32) + sizeof(quint64) + sizeof(quint64) + sizeof(quint8) * 16 + sizeof(quint8) + sizeof(quint8))
        return nullptr;

    quint8 *dp_inner_pointer = (quint8 *)dp + sizeof(struct qefi_device_path_header);
    quint32 partitionNumber = qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 start = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 size = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint8 *signature = dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8) * 16;
    quint8 format = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    quint8 signatureType = *dp_inner_pointer;
    dp_inner_pointer += sizeof(quint8);
    return new QEFIDevicePathMediaHD(partitionNumber, start, size, signature, format, signatureType);
}

QEFIDevicePath *qefi_parse_dp_media_cdrom(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_CDROM)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE + sizeof(quint32) + sizeof(quint64) + sizeof(quint64))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 entry = qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 partitionRba = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 sectors = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathMediaCDROM(entry, partitionRba, sectors);
}

QEFIDevicePath *qefi_parse_dp_media_vendor(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_Vendor)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE + sizeof(quint8) * 16)
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUuid vendorGuid = qefi_format_guid(dp_inner_pointer);
    dp_inner_pointer += 16 * sizeof(quint8);
    QByteArray vendorData((char *)dp_inner_pointer, length - (dp_inner_pointer - (quint8 *)dp));
    return new QEFIDevicePathMediaVendor(vendorGuid, vendorData);
}

QEFIDevicePath *qefi_parse_dp_media_protocol(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_Protocol)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE + sizeof(quint8) * 16)
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QUuid protocolGuid = qefi_format_guid(dp_inner_pointer);
    return new QEFIDevicePathMediaProtocol(protocolGuid);
}

QEFIDevicePath *qefi_parse_dp_media_firmware_file(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_FirmwareFile)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QByteArray piInfo((char *)dp_inner_pointer, length - (dp_inner_pointer - (quint8 *)dp));
    return new QEFIDevicePathMediaFirmwareFile(piInfo);
}

QEFIDevicePath *qefi_parse_dp_media_fv(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    QByteArray piInfo((char *)dp_inner_pointer, length - (dp_inner_pointer - (quint8 *)dp));
    return new QEFIDevicePathMediaFirmwareVolume(piInfo);
}

QEFIDevicePath *qefi_parse_dp_media_relative_offset(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_RelativeOffset)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE + sizeof(quint32) + sizeof(quint64) + sizeof(quint64))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint32 reserved = qFromLittleEndian<quint32>(*((quint32 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint32);
    quint64 firstByte = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 lastByte = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    return new QEFIDevicePathMediaRelativeOffset(reserved, firstByte, lastByte);
}

QEFIDevicePath *qefi_parse_dp_media_ramdisk(struct qefi_device_path_header *dp, int dp_size)
{
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE)
        return nullptr;
    if (dp->type != QEFIDevicePathType::DP_Media || dp->subtype != QEFIDevicePathMediaSubType::MEDIA_RamDisk)
        return nullptr;
    int length = qefi_dp_length(dp);
    if (length != dp_size || length <= 0)
        return nullptr;

    // Check size
    if (dp_size < QEFI_DEVICE_PATH_HEADER_SIZE + sizeof(quint64) + sizeof(quint64) + sizeof(quint8) * 16 + sizeof(quint16))
        return nullptr;

    quint8 *dp_inner_pointer = ((quint8 *)dp) + sizeof(struct qefi_device_path_header);
    quint64 startAddress = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    quint64 endAddress = qFromLittleEndian<quint64>(*((quint64 *)dp_inner_pointer));
    dp_inner_pointer += sizeof(quint64);
    QUuid diskTypeGuid = qefi_format_guid(dp_inner_pointer);
    dp_inner_pointer += sizeof(quint8) * 16;
    quint16 instanceNumber = qFromLittleEndian<quint16>(*((quint16 *)dp_inner_pointer));
    return new QEFIDevicePathMediaRAMDisk(startAddress, endAddress, diskTypeGuid, instanceNumber);
}

// Media formating
QByteArray qefi_format_dp_media_hdd(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_HD)
        return QByteArray();
    QEFIDevicePathMediaHD *dp_instance = dynamic_cast<QEFIDevicePathMediaHD *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 partitionNumber = qToLittleEndian<quint32>(dp_instance->partitionNumber());
    buffer.append((const char *)&partitionNumber, sizeof(quint32));
    quint64 start = qToLittleEndian<quint64>(dp_instance->start());
    buffer.append((const char *)&start, sizeof(quint64));
    quint64 size = qToLittleEndian<quint64>(dp_instance->size());
    buffer.append((const char *)&size, sizeof(quint64));
    const quint8 *signature = dp_instance->rawSignature();
    buffer.append((const char *)signature, sizeof(quint8) * 16);
    buffer.append((const char)(dp_instance->format()));
    buffer.append((const char)(dp_instance->signatureType()));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_media_file(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_File)
        return QByteArray();
    QEFIDevicePathMediaFile *dp_instance = dynamic_cast<QEFIDevicePathMediaFile *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(qefi_format_string_to_ucs2(dp_instance->name(), true));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_media_cdrom(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_CDROM)
        return QByteArray();
    QEFIDevicePathMediaCDROM *dp_instance = dynamic_cast<QEFIDevicePathMediaCDROM *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 bootCatalogEntry = qToLittleEndian<quint32>(dp_instance->bootCatalogEntry());
    buffer.append((const char *)&bootCatalogEntry, sizeof(quint32));
    quint64 partitionRba = qToLittleEndian<quint64>(dp_instance->partitionRba());
    buffer.append((const char *)&partitionRba, sizeof(quint64));
    quint64 sectors = qToLittleEndian<quint64>(dp_instance->sectors());
    buffer.append((const char *)&sectors, sizeof(quint64));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_media_vendor(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_Vendor)
        return QByteArray();
    QEFIDevicePathMediaVendor *dp_instance = dynamic_cast<QEFIDevicePathMediaVendor *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

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

QByteArray qefi_format_dp_media_protocol(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_Protocol)
        return QByteArray();
    QEFIDevicePathMediaProtocol *dp_instance = dynamic_cast<QEFIDevicePathMediaProtocol *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(qefi_rfc4122_to_guid(dp_instance->protocolGuid().toRfc4122()));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_media_firmware_file(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_FirmwareFile)
        return QByteArray();
    QEFIDevicePathMediaFirmwareFile *dp_instance = dynamic_cast<QEFIDevicePathMediaFirmwareFile *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->piInfo());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_media_fv(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume)
        return QByteArray();
    QEFIDevicePathMediaFirmwareVolume *dp_instance = dynamic_cast<QEFIDevicePathMediaFirmwareVolume *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    buffer.append(dp_instance->piInfo());

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_media_relative_offset(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_RelativeOffset)
        return QByteArray();
    QEFIDevicePathMediaRelativeOffset *dp_instance = dynamic_cast<QEFIDevicePathMediaRelativeOffset *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint32 reserved = qToLittleEndian<quint32>(dp_instance->reserved());
    buffer.append((const char *)&reserved, sizeof(quint32));
    quint64 firstByte = qToLittleEndian<quint64>(dp_instance->firstByte());
    buffer.append((const char *)&firstByte, sizeof(quint64));
    quint64 lastByte = qToLittleEndian<quint64>(dp_instance->lastByte());
    buffer.append((const char *)&lastByte, sizeof(quint64));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

QByteArray qefi_format_dp_media_ramdisk(QEFIDevicePath *dp)
{
    if (dp->type() != QEFIDevicePathType::DP_Media || dp->subType() != QEFIDevicePathMediaSubType::MEDIA_RamDisk)
        return QByteArray();
    QEFIDevicePathMediaRAMDisk *dp_instance = dynamic_cast<QEFIDevicePathMediaRAMDisk *>(dp);
    if (dp_instance == nullptr)
        return QByteArray();

    QByteArray buffer;
    // Append the types
    buffer.append(dp->type());
    buffer.append(dp->subType());
    // Append the basic length
    buffer.append((char)4);
    buffer.append((char)0);
    // Append the fields
    quint64 startAddress = qToLittleEndian<quint64>(dp_instance->startAddress());
    buffer.append((const char *)&startAddress, sizeof(quint64));
    quint64 endAddress = qToLittleEndian<quint64>(dp_instance->endAddress());
    buffer.append((const char *)&endAddress, sizeof(quint64));
    buffer.append(qefi_rfc4122_to_guid(dp_instance->diskTypeGuid().toRfc4122()));
    quint16 instanceNumber = qToLittleEndian<quint16>(dp_instance->instanceNumber());
    buffer.append((const char *)&instanceNumber, sizeof(quint16));

    // Fix the length
    quint16 len = (buffer.size() & 0xFFFF);
    buffer[2] = (len & 0xFF);
    buffer[3] = (len >> 8);

    return buffer;
}

// Subclasses for media
QEFIDevicePathMediaHD::QEFIDevicePathMediaHD(quint32 partitionNumber, quint64 start, quint64 size, quint8 *signature, quint8 format, quint8 signatureType)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_HD)
    , m_partitionNumber(partitionNumber)
    , m_start(start)
    , m_size(size)
    , m_format(format)
    , m_signatureType(signatureType)
    , m_gptGuid()
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
        m_mbrSignature = (quint32)m_signature[0] | (quint32)m_signature[1] << 8 | (quint32)m_signature[2] << 16 | (quint32)m_signature[3] << 24;
        break;
    }
}

QEFIDevicePathMediaCDROM::QEFIDevicePathMediaCDROM(quint32 entry, quint64 partitionRba, quint64 sectors)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_CDROM)
    , m_bootCatalogEntry(entry)
    , m_partitionRba(partitionRba)
    , m_sectors(sectors)
{
}

QEFIDevicePathMediaVendor::QEFIDevicePathMediaVendor(QUuid vendorGuid, QByteArray vendorData)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_Vendor)
    , m_vendorGuid(vendorGuid)
    , m_vendorData(vendorData)
{
}

QEFIDevicePathMediaFile::QEFIDevicePathMediaFile(QString name)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_File)
    , m_name(name)
{
}

QString QEFIDevicePathMediaFile::name() const
{
    return m_name;
}

QEFIDevicePathMediaProtocol::QEFIDevicePathMediaProtocol(QUuid protocolGuid)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_Protocol)
    , m_protocolGuid(protocolGuid)
{
}

QEFIDevicePathMediaFirmwareFile::QEFIDevicePathMediaFirmwareFile(QByteArray piInfo)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_FirmwareFile)
    , m_piInfo(piInfo)
{
}

QEFIDevicePathMediaFirmwareVolume::QEFIDevicePathMediaFirmwareVolume(QByteArray piInfo)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume)
    , m_piInfo(piInfo)
{
}

QEFIDevicePathMediaRelativeOffset::QEFIDevicePathMediaRelativeOffset(quint32 reserved, quint64 firstByte, quint64 lastByte)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_RelativeOffset)
    , m_reserved(reserved)
    , m_firstByte(firstByte)
    , m_lastByte(lastByte)
{
}

QEFIDevicePathMediaRAMDisk::QEFIDevicePathMediaRAMDisk(quint64 startAddress, quint64 endAddress, QUuid diskTypeGuid, quint16 instanceNumber)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_RamDisk)
    , m_startAddress(startAddress)
    , m_endAddress(endAddress)
    , m_diskTypeGuid(diskTypeGuid)
    , m_instanceNumber(instanceNumber)
{
}
