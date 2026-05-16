#ifndef QEFI_P_H
#define QEFI_P_H

#include <QtCore/qglobal.h>
#include <QtEndian>
#include <QByteArray>
#include <QString>
#include <QLoggingCategory>
#include <cstring>

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

#endif // QEFI_P_H
