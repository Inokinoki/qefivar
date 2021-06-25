#include "qefi.h"

#ifdef Q_OS_WIN
/* Implementation based on Windows API */
#include <Windows.h>
#include <WinBase.h>
#include <tchar.h>
#include <wbemidl.h>

bool qefi_is_available()
{
    FIRMWARE_TYPE fType;
    BOOL status = GetFirmwareType(&fType);
    return status == 0 && fType == FirmwareTypeUefi;
}

DWORD read_efivar_win(LPCSTR name, LPCSTR uuid, LPSTR buffer, DWORD size)
{
    DWORD len = GetFirmwareEnvironmentVariable(name, uuid, buffer, size);
    if (len == 0)
    {
        DWORD errorCode = GetLastError();
        // std::cerr << "Error reading : code " << errorCode << "\n";
    }
    return len;
}

#define EFIVAR_BUFFER_SIZE 4096

quint16 qefi_get_variable_uint16(QUuid uuid, QString name)
{
    std::string std_name = name.toStdString();
    const char *c_name = std_name.c_str();
    std::string std_uuid = uuid.toString(QUuid::WithBraces).toStdString();
    const char *c_uuid = std_uuid.c_str();

    // Create a buffer
    char buffer[EFIVAR_BUFFER_SIZE];

    size_t length = read_efivar_win(c_name, c_uuid, buffer, EFIVAR_BUFFER_SIZE);

    if (length < 2)
    {
        return 0;
    }

    // Read as uint16, platform-independant
    quint16 value = *((quint16 *)buffer);

    return value;
}

QByteArray qefi_get_variable(QUuid uuid, QString name)
{
    std::string std_name = name.toStdString();
    const char *c_name = std_name.c_str();
    std::string std_uuid = uuid.toString(QUuid::WithBraces).toStdString();
    const char *c_uuid = std_uuid.c_str();

    // Create a buffer
    char buffer[EFIVAR_BUFFER_SIZE];

    size_t length = read_efivar_win(c_name, c_uuid, buffer, EFIVAR_BUFFER_SIZE);

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
    // TODO
}

void qefi_set_variable(QUuid uuid, QString name, QByteArray value)
{
    // TODO
}

#else
/* Implementation based on libefivar */
extern "C" {
#include <efivar/efivar.h>
#include <efivar/efiboot.h>
}

bool qefi_is_available()
{
    return efi_variables_supported();
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
    if (return_code != 0)
    {
        value = 0;
    }
    else
    {
        // Read as uint16, platform-independant
        value = *((quint16 *)data);
        free(data);
    }

    return value;
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
    if (return_code != 0)
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

void qefi_set_variable_uint16(QUuid uuid, QString name, quint16 value);
void qefi_set_variable(QUuid uuid, QString name, QByteArray value);
#endif
