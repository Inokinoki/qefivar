#include "qefi.h"

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
QString qefi_extract_name(QByteArray data)
{
    QString entry_name;
    if (data.size() > sizeof(struct qefi_load_option_header)) {
        struct qefi_load_option_header *header =
            (struct qefi_load_option_header *)data.data();
        quint16 *c = (quint16*)data.data();
        quint16 dp_list_length = qFromLittleEndian<quint16>(header->path_list_length);
        c = c + sizeof(struct qefi_load_option_header) / 2;
        for (int index = sizeof(struct qefi_load_option_header);
            index < data.size() - dp_list_length - 2;   // Exclude the last 0
            index += 2, c++)
        {
            if (*c == 0) break;
            entry_name.append(*c);
        }
    }
    return entry_name;
}

QString qefi_extract_path(QByteArray data)
{
    QString path;
    if (data.size() > sizeof(struct qefi_load_option_header)) {
        struct qefi_load_option_header *header =
            (struct qefi_load_option_header *)data.data();
        quint16 *c = (quint16*)data.data();
        quint16 dp_list_length = qFromLittleEndian<quint16>(header->path_list_length);

        c = c + sizeof(struct qefi_load_option_header) / 2;
        for (int index = sizeof(struct qefi_load_option_header);
            index < data.size() - dp_list_length - 2;   // Exclude the last 0
            index += 2, c++)
        {
            // Find the end of description
            if (*c == 0) break;
        }

        // Keep the remainder length
        qint32 remainder_length = dp_list_length;
        quint8 *list_pointer = ((quint8 *)(c + 1));
        while (remainder_length > 0) {
            struct qefi_device_path_header *dp_header =
                (struct qefi_device_path_header *)list_pointer;
            quint16 length = qFromLittleEndian<quint16>(dp_header->length);
            if (dp_header->type == DP_Media && dp_header->subtype == MEDIA_File) {
                // Media File
                c = (quint16 *)(list_pointer + sizeof(struct qefi_device_path_header));
                for (int index = 0; index < length -
                    sizeof(struct qefi_device_path_header) - 2; index += 2, c++)
                {
                    path.append(*c);
                }
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

QEFILoadOption::QEFILoadOption(QByteArray &bootData)
    : m_bootData(bootData)
{
    if (m_bootData.size() > sizeof(struct qefi_load_option_header)) {
        struct qefi_load_option_header *header =
            (struct qefi_load_option_header *)m_bootData.data();
        m_isVisible = (qFromLittleEndian<quint32>(header->attributes)
            & QEFI_LOAD_OPTION_ACTIVE);
        m_name = qefi_extract_name(m_bootData);
        m_shortPath = qefi_extract_path(m_bootData);

        // TODO: Parse the device path if exists
    }
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

// Subclasses for media
QEFIDevicePathMediaHD::QEFIDevicePathMediaHD(quint32 partitionNumber,
    quint64 start, quint64 size, quint8 *signature,
    quint8 format, quint8 signatureType)
    : QEFIDevicePathMedia((quint8)QEFIDevicePathMediaSubType::MEDIA_HD),
    m_partitionNumber(partitionNumber), m_start(start), m_size(size),
    m_format(format), m_signatureType(signatureType)
{
    for (int i = 0; i < 16; i++) {
        m_signature[i] = signature[i];
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
