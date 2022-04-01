#ifndef QEFI_H
#define QEFI_H

#include <QtCore/qglobal.h>

#if defined(QEFI_LIBRARY)
#  define QEFI_EXPORT Q_DECL_EXPORT
#else
#  define QEFI_EXPORT Q_DECL_IMPORT
#endif

#include <QUuid>
#include <QString>
#include <QSharedPointer>

QEFI_EXPORT bool qefi_is_available();
QEFI_EXPORT bool qefi_has_privilege();

QEFI_EXPORT quint16 qefi_get_variable_uint16(QUuid uuid, QString name);
QEFI_EXPORT QByteArray qefi_get_variable(QUuid uuid, QString name);

QEFI_EXPORT void qefi_set_variable_uint16(QUuid uuid, QString name, quint16 value);
QEFI_EXPORT void qefi_set_variable(QUuid uuid, QString name, QByteArray value);

QEFI_EXPORT QString qefi_extract_name(QByteArray data);
QEFI_EXPORT QString qefi_extract_path(QByteArray data);

#define QEFI_LOAD_OPTION_ACTIVE		        0x00000001
#define QEFI_LOAD_OPTION_FORCE_RECONNECT	0x00000002
#define QEFI_LOAD_OPTION_HIDDEN		        0x00000008
#define QEFI_LOAD_OPTION_CATEGORY_MASK	    0x00001f00
#define QEFI_LOAD_OPTION_CATEGORY_BOOT	    0x00000000
#define QEFI_LOAD_OPTION_CATEGORY_APP	    0x00000100

class QEFIDevicePath;

class QEFIDevicePathHardware;
// TODO: Subclasses for hardware
class QEFIDevicePathACPI;
// TODO: Subclasses for ACPI
class QEFIDevicePathMessage;
// TODO: Subclasses for message
class QEFIDevicePathMedia;
// TODO: Subclasses for media
class QEFIDevicePathBIOSBoot;
// TODO: Subclasses for BIOSBoot

class QEFILoadOption
{
    bool m_isVisible;
    QString m_name;
    QString m_shortPath;
    QByteArray &m_bootData;
    QList<QSharedPointer<QEFIDevicePath>> m_devicePathList;
public:
    QEFILoadOption(QByteArray &bootData);
    virtual ~QEFILoadOption();
};

enum QEFIDevicePathType
{
    DP_Hardware    = 0x01,
    DP_ACPI        = 0x02,
    DP_Message     = 0x03,
    DP_Media       = 0x04,
    DP_BIOSBoot    = 0x05,
    /* Others */
    DP_End         = 0x7F
};

enum QEFIDevicePathHardwareSubType
{
    HW_PCI         = 0x01,
    HW_PCCard      = 0x02,
    HW_MMIO        = 0x03,
    HW_Vendor      = 0x04,
    HW_Controller  = 0x05,
    HW_BMC         = 0x06
};

enum QEFIDevicePathACPISubType
{
    ACPI_HID         = 0x01,
    ACPI_HIDEX       = 0x02,
    ACPI_ADR         = 0x03
};

enum QEFIDevicePathMessageSubType
{
    MSG_ATAPI       = 0x01,
    MSG_SCSI        = 0x02,
    MSG_FibreChan   = 0x03,
    MSG_Fibre1394   = 0x04,
    MSG_USB         = 0x05,

    MSG_I2O         = 0x06,
    MSG_InfiniBand  = 0x09,

    MSG_Vendor      = 0x0A,
    MSG_MACAddr     = 0x0B,
    MSG_IPv4        = 0x0C,
    MSG_IPv6        = 0x0D,
    MSG_UART        = 0x0E,
    MSG_USBClass    = 0x0F,

    MSG_USBWWID     = 0x10,

    MSG_LUN         = 0x11,
    MSG_SATA        = 0x12,

    MSG_VLAN        = 0x14,

    MSG_FibreChanEx = 0x15,

    MSG_SASEX       = 0x16,

    MSG_NVME        = 0x17,
    MSG_URI,
    MSG_UFS,
    MSG_SD, MSG_BT, MSG_WiFi, MSG_EMMC, MSG_BTLE, MSG_DNS, MSG_NVDIMM
    // 0x20
};

enum QEFIDevicePathMediaSubType
{
    MEDIA_HD = 0x01,
    MEDIA_CDROM,
    MEDIA_Vendor,
    MEDIA_File,
    MEDIA_Protocol,
    MEDIA_FirmwareFile,
    MEDIA_FirmwareVolume,
    MEDIA_RelativeOffset,
    MEDIA_RamDisk
    // 0x09
};

enum QEFIDevicePathBIOSBootSubType
{
    BIOS_BIOSBoot = 0x01
};

enum QEFIDevicePathEndSubType
{
    END_EndInstance = 0x01,
    END_End         = 0xFF
};

#endif // QEFI_H
