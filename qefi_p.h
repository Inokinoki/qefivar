#ifndef QEFI_P_H
#define QEFI_P_H

#include <QtCore/qglobal.h>
#include <QtEndian>
#include <QByteArray>
#include <QString>
#include <QLoggingCategory>
#include <cstring>

#include "qefi.h"

// Qt5/Qt6 compatibility macros
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6: Prefer C++17 [[deprecated]] when available
    #if __cplusplus >= 201703L
        #define QEFI_DEPRECATED_X(text) [[deprecated(text)]]
    #else
        #define QEFI_DEPRECATED_X(text) Q_DECL_DEPRECATED_X(text)
    #endif
#else
    // Qt5: Use Q_DECL_DEPRECATED_X
    #define QEFI_DEPRECATED_X(text) Q_DECL_DEPRECATED_X(text)
#endif

Q_DECLARE_LOGGING_CATEGORY(QEFI_LOG)

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

// Safe unaligned little-endian read (portable across all architectures)
template<typename T>
inline T qefi_read_le(const void *ptr) {
    T val;
    std::memcpy(&val, ptr, sizeof(T));
    return qFromLittleEndian(val);
}

// Safe unaligned write in little-endian
template<typename T>
inline void qefi_write_le(void *ptr, T value) {
    T le = qToLittleEndian(value);
    std::memcpy(ptr, &le, sizeof(T));
}

// Internal utility functions (implemented in qefi.cpp)
int qefi_dp_length(const struct qefi_device_path_header *dp_header);
int qefi_dp_count(struct qefi_device_path_header *dp_header_pointer, int max_dp_size);
int qefi_dp_total_size(struct qefi_device_path_header *dp_header_pointer, int max_dp_size);
QString qefi_parse_ucs2_string(quint8 *data, int max_size);
QByteArray qefi_format_string_to_ucs2(QString str, bool isEnd);

// Internal validation helpers (avoid deprecation warnings)
bool qefi_validate_load_option(const QByteArray &data);
int qefi_internal_description_length(const QByteArray &data);
int qefi_internal_dp_list_length(const QByteArray &data);
int qefi_internal_optional_data_length(const QByteArray &data);

// Generic device path dispatcher functions
QEFIDevicePath *qefi_parse_dp(struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp(QEFIDevicePath *dp);

// Hardware device path parse/format functions
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
QByteArray qefi_format_dp_hardware_pci(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_pccard(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_mmio(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_vendor(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_controller(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_bmc(QEFIDevicePath *dp);

// ACPI device path parse/format functions
QEFIDevicePath *qefi_parse_dp_acpi_hid(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_acpi_hidex(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_acpi_adr(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_acpi_hid(QEFIDevicePath *dp);
QByteArray qefi_format_dp_acpi_hidex(QEFIDevicePath *dp);
QByteArray qefi_format_dp_acpi_adr(QEFIDevicePath *dp);

// Message device path parse/format functions
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
QByteArray qefi_private_format_message_subtype(QEFIDevicePath *dp);
QEFIDevicePath *qefi_private_parse_message_subtype(
    struct qefi_device_path_header *dp, int dp_size);

// Media device path parse/format functions
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
QByteArray qefi_format_dp_media_hdd(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_file(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_cdrom(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_vendor(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_protocol(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_firmware_file(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_fv(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_relative_offset(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_ramdisk(QEFIDevicePath *dp);

#endif // QEFI_P_H
