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
        if (tempLength < 0) return tempLength;

        size += tempLength;
        count++;

        if (dp_header_pointer->type == QEFIDevicePathType::DP_End &&
            dp_header_pointer->subtype == 0xFF)
            break;

        dp_header_pointer = (struct qefi_device_path_header *)
            ((quint8 *)dp_header_pointer) + tempLength;
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
        if (tempLength < 0) return tempLength;

        size += tempLength;

        if (dp_header_pointer->type == QEFIDevicePathType::DP_End &&
            dp_header_pointer->subtype == 0xFF)
            break;

        dp_header_pointer = (struct qefi_device_path_header *)
            ((quint8 *)dp_header_pointer) + tempLength;
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

QEFIDevicePath *qefi_parse_dp_hardware_file(struct qefi_device_path_header *dp, int dp_size)
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

QEFIDevicePath *qefi_parse_dp_hardware_hdd(struct qefi_device_path_header *dp, int dp_size)
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

QEFIDevicePath *qefi_parse_dp(struct qefi_device_path_header *dp, int dp_size)
{
	quint8 type = dp->type, subtype = dp->subtype;
	int length = qefi_dp_length(dp);
    qDebug() << "Parsing DP: length " << length << " " <<
        "type" << type << "subtype" << subtype;
    if (length != dp_size || length <= 0) return nullptr;

    if (type == QEFIDevicePathType::DP_Hardware) {
        // TODO: Parse hardware
    } else if (type == QEFIDevicePathType::DP_ACPI) {
        // TODO: Parse DP_ACPI
    } else if (type == QEFIDevicePathType::DP_Message) {
        // TODO: Parse Message)
    } else if (type == QEFIDevicePathType::DP_Media) {
        // Parse Media
        switch (subtype) {
            case QEFIDevicePathMediaSubType::MEDIA_HD:
                qDebug() << "Parsing DP media HD";
                return qefi_parse_dp_hardware_hdd(dp, length);
            case QEFIDevicePathMediaSubType::MEDIA_File:
                qDebug() << "Parsing DP media file";
                return qefi_parse_dp_hardware_file(dp, length);
        }
    } else if (type == QEFIDevicePathType::DP_BIOSBoot) {
        // TODO: Parse BIOSBoot
    }
    return nullptr;
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
QString qefi_extract_name(QByteArray data)
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

QString qefi_extract_path(QByteArray data)
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
                c = (quint16 *)(list_pointer + sizeof(struct qefi_device_path_header));
                for (int index = 0; index < length -
                    sizeof(struct qefi_device_path_header) - 2; index += 2, c++) {
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

QByteArray &QEFILoadOption::bootData() const
{
    return m_bootData;
}

QList<QSharedPointer<QEFIDevicePath> > QEFILoadOption::devicePathList() const
{
    return m_devicePathList;
}

QEFILoadOption::QEFILoadOption(QByteArray &bootData)
    : m_bootData(bootData), m_isValidated(false)
{
    if (qefi_loadopt_is_valid(bootData)) {
        struct qefi_load_option_header *header =
            (struct qefi_load_option_header *)m_bootData.data();
        m_isVisible = (qFromLittleEndian<quint32>(header->attributes)
            & QEFI_LOAD_OPTION_ACTIVE);
        m_name = qefi_extract_name(m_bootData);
        m_shortPath = qefi_extract_path(m_bootData);

        m_isValidated = true;

        // Parse the device path if exists
        int dp_list_length = qefi_loadopt_dp_list_length(m_bootData);
        if (dp_list_length >= 0) {
            int dp_infered_length = m_bootData.size() -     // Optional + DP
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
