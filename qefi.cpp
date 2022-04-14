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

// TODO: Test it
QByteArray qefi_format_string_to_ucs2(QString str, bool isEnd)
{
    QByteArray utf8 = str.toUtf8();
    QByteArray ucs2;

    for (int i = 0, j = 0; i < utf8.size() && utf8[i] != '\0'; j++) {
        quint16 val16;
        quint32 val = 0;

        if ((utf8[i] & 0xe0) == 0xe0 && !(utf8[i] & 0x10)) {
            val = ((utf8[i+0] & 0x0f) << 10)
                | ((utf8[i+1] & 0x3f) << 6)
                | ((utf8[i+2] & 0x3f) << 0);
            i += 3;
        } else if ((utf8[i] & 0xc0) == 0xc0 && !(utf8[i] & 0x20)) {
            val = ((utf8[i+0] & 0x1f) << 6) | ((utf8[i+1] & 0x3f) << 0);
            i += 2;
        } else {
            val = utf8[i] & 0x7f;
            i += 1;
        }
        val16 = (val & 0xFFFF);

        // Append L8
        ucs2.append((char)(val16 & 0xFF));
        // Append H8
        ucs2.append((char)((val16 >> 8) & 0xFF));
    }
    if (isEnd) {
        // Append \0 terminator
        ucs2.append((char)0x00);
        ucs2.append((char)0x00);
    }
    return ucs2;
}

// Hardware parsing
QEFIDevicePath *qefi_parse_dp_hardware_pci(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_hardware_pccard(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_hardware_mmio(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_hardware_vendor(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_hardware_controller(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_hardware_bmc(
    struct qefi_device_path_header *dp, int dp_size);

// ACPI parsing
QEFIDevicePath *qefi_parse_dp_acpi_hid(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_acpi_hidex(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_acpi_adr(
    struct qefi_device_path_header *dp, int dp_size);

// Message parsing
QEFIDevicePath *qefi_parse_dp_message_atapi(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_scsi(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_fibre_chan(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_1394(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_usb(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_i2o(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_infiniband(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_vendor(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_mac_addr(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_ipv4(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_ipv6(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_uart(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_usb_class(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_usb_wwid(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_lun(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_sata(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_iscsi(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_vlan(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_fibre_chan_ex(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_sas_ex(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_nvme(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_uri(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_ufs(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_sd(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_bt(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_wifi(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_emmc(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_btle(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_dns(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_message_nvdimm(
    struct qefi_device_path_header *dp, int dp_size);

// Media parsing
QEFIDevicePath *qefi_parse_dp_media_file(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_hdd(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_cdrom(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_vendor(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_protocol(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_firmware_file(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_fv(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_relative_offset(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_parse_dp_media_ramdisk(
    struct qefi_device_path_header *dp, int dp_size);

QEFIDevicePath *qefi_private_parse_message_subtype(
    struct qefi_device_path_header *dp, int dp_size);

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

// Format Hardware
QByteArray qefi_format_dp_hardware_pci(QEFIDevicePath *dp);

QByteArray qefi_format_dp_hardware_pccard(QEFIDevicePath *dp);

QByteArray qefi_format_dp_hardware_mmio(QEFIDevicePath *dp);

QByteArray qefi_format_dp_hardware_vendor(QEFIDevicePath *dp);

QByteArray qefi_format_dp_hardware_controller(QEFIDevicePath *dp);

QByteArray qefi_format_dp_hardware_bmc(QEFIDevicePath *dp);

// Format ACPI
QByteArray qefi_format_dp_acpi_hid(QEFIDevicePath *dp);

QByteArray qefi_format_dp_acpi_hidex(QEFIDevicePath *dp);

QByteArray qefi_format_dp_acpi_adr(QEFIDevicePath *dp);

// Format Message
QByteArray qefi_private_format_message_subtype(QEFIDevicePath *dp);

// Format Media
QByteArray qefi_format_dp_media_hdd(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_file(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_cdrom(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_vendor(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_protocol(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_firmware_file(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_fv(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_relative_offset(QEFIDevicePath *dp);

QByteArray qefi_format_dp_media_ramdisk(QEFIDevicePath *dp);


QByteArray qefi_format_dp(QEFIDevicePath *dp)
{
    QEFIDevicePathType type = dp->type();
    quint8 subtype = dp->subType();
    qDebug() << "Formating DP: type" << type << "subtype" << subtype;

    if (type == QEFIDevicePathType::DP_Hardware) {
        // Format hardware
        switch (subtype) {
            case QEFIDevicePathHardwareSubType::HW_PCI:
                qDebug() << "Formating DP hardware PCI";
                return qefi_format_dp_hardware_pci(dp);
            case QEFIDevicePathHardwareSubType::HW_PCCard:
                qDebug() << "Formating DP hardware PCCard";
                return qefi_format_dp_hardware_pccard(dp);
            case QEFIDevicePathHardwareSubType::HW_MMIO:
                qDebug() << "Formating DP hardware MMIO";
                return qefi_format_dp_hardware_mmio(dp);
            case QEFIDevicePathHardwareSubType::HW_Vendor:
                qDebug() << "Formating DP hardware Vendor";
                return qefi_format_dp_hardware_vendor(dp);
            case QEFIDevicePathHardwareSubType::HW_Controller:
                qDebug() << "Formating DP hardware Controller";
                return qefi_format_dp_hardware_controller(dp);
            case QEFIDevicePathHardwareSubType::HW_BMC:
                qDebug() << "Formating DP hardware BMC";
                return qefi_format_dp_hardware_bmc(dp);
        }
    } else if (type == QEFIDevicePathType::DP_ACPI) {
        // Format DP_ACPI
        switch (subtype) {
            case QEFIDevicePathACPISubType::ACPI_HID:
                qDebug() << "Formating DP ACPI HID";
                return qefi_format_dp_acpi_hid(dp);
            case QEFIDevicePathACPISubType::ACPI_HIDEX:
                qDebug() << "Formating DP ACPI HIDEX";
                return qefi_format_dp_acpi_hidex(dp);
            case QEFIDevicePathACPISubType::ACPI_ADR:
                qDebug() << "Formating DP ACPI ADR";
                return qefi_format_dp_acpi_adr(dp);
        }
    } else if (type == QEFIDevicePathType::DP_Message) {
        // Format Message
        return qefi_private_format_message_subtype(dp);
    } else if (type == QEFIDevicePathType::DP_Media) {
        // Format Media
        switch (subtype) {
            case QEFIDevicePathMediaSubType::MEDIA_HD:
                qDebug() << "Formating DP media HD";
                return qefi_format_dp_media_hdd(dp);
            case QEFIDevicePathMediaSubType::MEDIA_File:
                qDebug() << "Formating DP media file";
                return qefi_format_dp_media_file(dp);
            case QEFIDevicePathMediaSubType::MEDIA_CDROM:
                qDebug() << "Formating DP media CDROM";
                return qefi_format_dp_media_cdrom(dp);
            case QEFIDevicePathMediaSubType::MEDIA_Vendor:
                qDebug() << "Formating DP media vendor";
                return qefi_format_dp_media_vendor(dp);
            case QEFIDevicePathMediaSubType::MEDIA_Protocol:
                qDebug() << "Formating DP media protocol";
                return qefi_format_dp_media_protocol(dp);
            case QEFIDevicePathMediaSubType::MEDIA_FirmwareFile:
                qDebug() << "Formating DP media firmware file";
                return qefi_format_dp_media_firmware_file(dp);
            case QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume:
                qDebug() << "Formating DP media FV";
                return qefi_format_dp_media_fv(dp);
            case QEFIDevicePathMediaSubType::MEDIA_RelativeOffset:
                qDebug() << "Formating DP media relative offset";
                return qefi_format_dp_media_relative_offset(dp);
            case QEFIDevicePathMediaSubType::MEDIA_RamDisk:
                qDebug() << "Formating DP media ramdisk";
                return qefi_format_dp_media_ramdisk(dp);
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

QByteArray QEFILoadOption::optionalData() const
{
    return m_optionalData;
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

        // Optional data
        int optionalDataBegin = sizeof(struct qefi_load_option_header) +
            (m_name.length() + 1) * 2 + dp_list_length;
        if (optionalDataBegin < bootData.size()) {
            m_optionalData = QByteArray(((const char *)header) +
                sizeof(struct qefi_load_option_header) +
                (m_name.length() + 1) * 2 + dp_list_length,
                bootData.size() - optionalDataBegin);
        }
    }
    return m_isValidated;
}

// TODO: Add a test for this
QByteArray QEFILoadOption::format()
{
    QByteArray loadOptionData;

    struct qefi_load_option_header header;
    header.attributes = qToLittleEndian<quint32>(
        m_isVisible ? QEFI_LOAD_OPTION_ACTIVE : 0);
    // TODO: Parse and format DP
    header.path_list_length = 0;

    // Encode name
    QByteArray name = qefi_format_string_to_ucs2(m_name, true);
    if (!name.size()) name.append(2, '\0');

    // Append header
    loadOptionData.append((const char *)&header,
        sizeof(struct qefi_load_option_header));
    // Append name
    loadOptionData.append(name);
    // Append DP
    for (QList<QSharedPointer<QEFIDevicePath> >::iterator i = m_devicePathList.begin();
         i != m_devicePathList.end(); i++) {
        loadOptionData.append(qefi_format_dp((*i).get()));
    }
    // Append the end of DP
    loadOptionData.append((char)QEFIDevicePathType::DP_End);
    loadOptionData.append((char)0xff);
    loadOptionData.append((char)0x04);
    loadOptionData.append((char)0x00);
    // Append Optional data
    loadOptionData.append(m_optionalData);

    // Never return invalidated data
    if (!qefi_loadopt_is_valid(loadOptionData)) return QByteArray();
    return loadOptionData;
}

QEFILoadOption::~QEFILoadOption()
{
    m_devicePathList.clear();
}

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
