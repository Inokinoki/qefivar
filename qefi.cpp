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
                QScopedPointer<QEFIDevicePath> dp(
                    qefi_parse_dp_hardware_file(dp_header, length));
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

QList<QSharedPointer<QEFIDevicePath> > QEFILoadOption::devicePathList() const
{
    return m_devicePathList;
}

QEFILoadOption::QEFILoadOption(QByteArray &bootData)
    : m_isValidated(false)
{
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
            // TODO: Create a standalone function to create UUID
            m_gptGuid = QUuid((uint)((uint)m_signature[0] |
                    (uint)m_signature[1] << 8 |
                    (uint)m_signature[2] << 16 |
                    (uint)m_signature[3] << 24),    // LE
                (ushort)m_signature[4] | (ushort)m_signature[5] << 8, // LE
                (ushort)m_signature[6] | (ushort)m_signature[7] << 8, // LE
                (uchar)m_signature[8], (uchar)m_signature[9],   // BE
                (uchar)m_signature[10], (uchar)m_signature[11],
                (uchar)m_signature[12], (uchar)m_signature[13],
                (uchar)m_signature[14], (uchar)m_signature[15]);
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
