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

/* Do not create this base class directly */
class QEFIDevicePath {
protected:
    enum QEFIDevicePathType m_type;
    quint8 m_subType;

    QEFIDevicePath(enum QEFIDevicePathType type, quint8 subType)
        : m_type(type), m_subType(subType) {}
public:
    virtual ~QEFIDevicePath() {}
};

class QEFIDevicePathHardware : public QEFIDevicePath
{
protected:
    QEFIDevicePathHardware(quint8 subType)
        : QEFIDevicePath(QEFIDevicePathType::DP_Hardware, subType) {}
public:
    virtual ~QEFIDevicePathHardware() {}
};

class QEFIDevicePathACPI : public QEFIDevicePath
{
protected:
    QEFIDevicePathACPI(quint8 subType)
        : QEFIDevicePath(QEFIDevicePathType::DP_ACPI, subType) {}
public:
    virtual ~QEFIDevicePathACPI() {}
};

class QEFIDevicePathMessage : public QEFIDevicePath
{
protected:
    QEFIDevicePathMessage(quint8 subType)
        : QEFIDevicePath(QEFIDevicePathType::DP_Message, subType) {}
public:
    virtual ~QEFIDevicePathMessage() {}
};

class QEFIDevicePathMedia : public QEFIDevicePath
{
protected:
    QEFIDevicePathMedia(quint8 subType)
        : QEFIDevicePath(QEFIDevicePathType::DP_Media, subType) {}
public:
    virtual ~QEFIDevicePathMedia() {}
};

class QEFIDevicePathBIOSBoot : public QEFIDevicePath
{
protected:
    QEFIDevicePathBIOSBoot(quint8 subType)
        : QEFIDevicePath(QEFIDevicePathType::DP_BIOSBoot, subType) {}
public:
    virtual ~QEFIDevicePathBIOSBoot() {}
};

// Load option
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

// Subclasses for hardware
class QEFIDevicePathHardwarePCI : public QEFIDevicePathHardware {
protected:
    quint8 m_function;
    quint8 m_device;
public:
    QEFIDevicePathHardwarePCI(quint8 function, quint8 device);
};

class QEFIDevicePathHardwarePCCard : public QEFIDevicePathHardware {
protected:
    quint8 m_function;
public:
    QEFIDevicePathHardwarePCCard(quint8 function);
};

class QEFIDevicePathHardwareMMIO : public QEFIDevicePathHardware {
protected:
    quint32 m_memoryType;
	quint64 m_startingAddress;
	quint64 m_endingAddress;
public:
    QEFIDevicePathHardwareMMIO(quint32 memoryType,
        quint64 startingAddress, quint64 endingAddress);
};

class QEFIDevicePathHardwareVendor : public QEFIDevicePathHardware {
protected:
    QUuid m_vendorGuid;
    QByteArray m_vendorData;
public:
    QEFIDevicePathHardwareVendor(QUuid vendorGuid, QByteArray vendorData);
};

class QEFIDevicePathHardwareController : public QEFIDevicePathHardware {
protected:
    quint32 m_controller;
public:
    QEFIDevicePathHardwareController(quint32 controller);
};

class QEFIDevicePathHardwareBMC : public QEFIDevicePathHardware {
protected:
	quint8 m_interfaceType;
	quint64 m_baseAddress;
public:
    QEFIDevicePathHardwareBMC(quint8 interfaceType, quint64 baseAddress);
};

// Subclasses for ACPI
class QEFIDevicePathACPIHID : public QEFIDevicePathACPI {
protected:
	quint32 m_hid;
	quint32 m_uid;
public:
    QEFIDevicePathACPIHID(quint32 hid, quint32 uid);
};

class QEFIDevicePathACPIHIDEX : public QEFIDevicePathACPI {
protected:
	quint32 m_hid;
	quint32 m_uid;
	quint32 m_cid;
    QString m_hidString;
    QString m_uidString;
    QString m_cidString;
public:
    QEFIDevicePathACPIHIDEX(quint32 hid, quint32 uid,
        quint32 cid, QString hidString,
        QString uidString, QString cidString);
};

class QEFIDevicePathACPIADR : public QEFIDevicePathACPI {
protected:
	QList<quint32> m_addresses;
public:
    QEFIDevicePathACPIADR(QList<quint32> addresses);
};
// TODO: Decode Device Path ACPI Address

// TODO: Subclasses for message

// Subclasses for media
class QEFIDevicePathMediaHD : public QEFIDevicePathMedia {
protected:
    quint32 m_partitionNumber;
    quint64 m_start;
	quint64 m_size;
	quint8 m_signature[16];
	quint8 m_format;
	quint8 m_signatureType;
public:
    QEFIDevicePathMediaHD(quint32 partitionNumber, quint64 start, quint64 size,
        quint8 *signature, quint8 format, quint8 signatureType);
};

class QEFIDevicePathMediaCDROM : public QEFIDevicePathMedia {
protected:
    quint32 m_bootCatalogEntry;
    quint64 m_partitionRba;
	quint64 m_sectors;
public:
    QEFIDevicePathMediaCDROM(quint32 entry, quint64 partitionRba, quint64 sectors);
};

class QEFIDevicePathMediaVendor : public QEFIDevicePathMedia {
protected:
    QUuid m_vendorGuid;
    QByteArray m_vendorData;
public:
    QEFIDevicePathMediaVendor(QUuid vendorGuid, QByteArray vendorData);
};

class QEFIDevicePathMediaFile : public QEFIDevicePathMedia {
protected:
    QString m_name;
public:
    QEFIDevicePathMediaFile(QString name);
};

class QEFIDevicePathMediaProtocol : public QEFIDevicePathMedia {
protected:
    QUuid m_protocolGuid;
public:
    QEFIDevicePathMediaProtocol(QUuid protocolGuid);
};

class QEFIDevicePathMediaFirmwareFile : public QEFIDevicePathMedia {
protected:
    QByteArray m_piInfo;
public:
    QEFIDevicePathMediaFirmwareFile(QByteArray piInfo);
};

class QEFIDevicePathMediaFirmwareVolume : public QEFIDevicePathMedia {
protected:
    QByteArray m_piInfo;
public:
    QEFIDevicePathMediaFirmwareVolume(QByteArray piInfo);
};

class QEFIDevicePathMediaRelativeOffset : public QEFIDevicePathMedia {
protected:
    quint32 m_reserved;
    quint64 m_firstByte;
    quint64 m_lastByte;
public:
    QEFIDevicePathMediaRelativeOffset(quint32 reserved,
        quint64 firstByte, quint64 lastByte);
};

class QEFIDevicePathMediaRAMDisk : public QEFIDevicePathMedia {
protected:
    quint64 m_startAddress;
    quint64 m_endAddress;
    QUuid m_disktTypeGuid;
    quint16 m_instanceNumber;
public:
    QEFIDevicePathMediaRAMDisk(quint64 startAddress, quint64 endAddress,
        QUuid disktTypeGuid, quint16 instanceNumber);
};

// TODO: Subclasses for BIOSBoot

#endif // QEFI_H
