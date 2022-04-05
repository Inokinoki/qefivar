#ifndef QEFI_H
#define QEFI_H

#include <QtCore/qglobal.h>

#if defined(QEFI_LIBRARY)
#  define QEFI_EXPORT Q_DECL_EXPORT
#else
#  define QEFI_EXPORT Q_DECL_IMPORT
#endif

#include <QUrl>
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

QEFI_EXPORT int qefi_loadopt_description_length(const QByteArray &data);
QEFI_EXPORT int qefi_loadopt_dp_list_length(const QByteArray &data);
QEFI_EXPORT int qefi_loadopt_optional_data_length(const QByteArray &data);
QEFI_EXPORT bool qefi_loadopt_is_valid(const QByteArray &data);

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
    MSG_1394        = 0x04,
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
    MSG_ISCSI       = 0x13,
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
    QEFIDevicePathType type() const { return m_type; }
    quint8 subType() const { return m_subType; }
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
    quint16 m_deviceType;
	quint16 m_status;
	QByteArray m_description;

public:
    QEFIDevicePathBIOSBoot(quint16 deviceType,
        quint16 status, QByteArray description)
        : QEFIDevicePath(QEFIDevicePathType::DP_BIOSBoot,
            QEFIDevicePathBIOSBootSubType::BIOS_BIOSBoot),
        m_deviceType(deviceType), m_status(status),
        m_description(description) {}
    virtual ~QEFIDevicePathBIOSBoot() {}
    quint16 deviceType() const { return m_deviceType; }
    quint16 status() const { return m_status; }
    QByteArray description() const { return m_description; }
};

// Load option
class QEFILoadOption
{
    bool m_isValidated;
    bool m_isVisible;
    QString m_name;
    QString m_shortPath;
    QList<QSharedPointer<QEFIDevicePath> > m_devicePathList;
public:
    QEFILoadOption(QByteArray &bootData);
    virtual ~QEFILoadOption();

    bool isValidated() const;

    QString name() const;
    bool isVisible() const;
    QString path() const;
    QList<QSharedPointer<QEFIDevicePath> > devicePathList() const;
};

// Subclasses for hardware
class QEFIDevicePathHardwarePCI : public QEFIDevicePathHardware {
protected:
    quint8 m_function;
    quint8 m_device;
public:
    QEFIDevicePathHardwarePCI(quint8 function, quint8 device);
    quint8 function() const { return m_function; }
    quint8 device() const { return m_device; }
};

class QEFIDevicePathHardwarePCCard : public QEFIDevicePathHardware {
protected:
    quint8 m_function;
public:
    QEFIDevicePathHardwarePCCard(quint8 function);
    quint8 function() const { return m_function; }
};

class QEFIDevicePathHardwareMMIO : public QEFIDevicePathHardware {
protected:
    quint32 m_memoryType;
	quint64 m_startingAddress;
	quint64 m_endingAddress;
public:
    QEFIDevicePathHardwareMMIO(quint32 memoryType,
        quint64 startingAddress, quint64 endingAddress);
    quint32 memoryType() const { return m_memoryType; }
    quint64 startingAddress() const { return m_startingAddress; }
    quint64 endingAddress() const { return m_endingAddress; }
};

class QEFIDevicePathHardwareVendor : public QEFIDevicePathHardware {
protected:
    QUuid m_vendorGuid;
    QByteArray m_vendorData;
public:
    QEFIDevicePathHardwareVendor(QUuid vendorGuid, QByteArray vendorData);
    QUuid vendorGuid() const { return m_vendorGuid; }
    QByteArray vendorData() const { return m_vendorData; }
};

class QEFIDevicePathHardwareController : public QEFIDevicePathHardware {
protected:
    quint32 m_controller;
public:
    QEFIDevicePathHardwareController(quint32 controller);
    quint32 controller() const { return m_controller; }
};

class QEFIDevicePathHardwareBMC : public QEFIDevicePathHardware {
protected:
	quint8 m_interfaceType;
	quint64 m_baseAddress;
public:
    QEFIDevicePathHardwareBMC(quint8 interfaceType, quint64 baseAddress);
    quint8 interfaceType() const { return m_interfaceType; }
    quint64 baseAddress() const { return m_baseAddress; }
};

// Subclasses for ACPI
class QEFIDevicePathACPIHID : public QEFIDevicePathACPI {
protected:
	quint32 m_hid;
	quint32 m_uid;
public:
    QEFIDevicePathACPIHID(quint32 hid, quint32 uid);
    quint32 hid() const { return m_hid; }
    quint32 uid() const { return m_uid; }
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
    quint32 hid() const { return m_hid; }
    quint32 uid() const { return m_uid; }
    quint32 cid() const { return m_cid; }
    QString hidString() const { return m_hidString; }
    QString uidString() const { return m_uidString; }
    QString cidString() const { return m_cidString; }
};

class QEFIDevicePathACPIADR : public QEFIDevicePathACPI {
protected:
	QList<quint32> m_addresses;
public:
    QEFIDevicePathACPIADR(QList<quint32> addresses);
};
// TODO: Decode Device Path ACPI Address

// Subclasses for message
class QEFIDevicePathMessageATAPI : public QEFIDevicePathMessage {
protected:
    quint8 m_primary;
    quint8 m_slave;
    quint16 m_lun;
public:
    QEFIDevicePathMessageATAPI(quint8 primary, quint8 slave, quint16 lun);
};

class QEFIDevicePathMessageSCSI : public QEFIDevicePathMessage {
protected:
    quint16 m_target;
    quint16 m_lun;
public:
    QEFIDevicePathMessageSCSI(quint16 m_target, quint16 lun);
};

class QEFIDevicePathMessageFibreChan : public QEFIDevicePathMessage {
protected:
    quint32 m_reserved;
    quint64 m_wwn;
    quint64 m_lun;
public:
    QEFIDevicePathMessageFibreChan(quint32 reserved, quint64 wwn,
        quint64 lun);
};

class QEFIDevicePathMessage1394 : public QEFIDevicePathMessage {
protected:
    quint32 m_reserved;
    quint64 m_guid;
public:
    QEFIDevicePathMessage1394(quint32 reserved, quint64 guid);
};

class QEFIDevicePathMessageUSB : public QEFIDevicePathMessage {
protected:
    quint8 m_parentPort;
    quint8 m_interface;
public:
    QEFIDevicePathMessageUSB(quint8 parentPort, quint8 interface);
};

class QEFIDevicePathMessageI2O : public QEFIDevicePathMessage {
protected:
    quint32 m_target;
public:
    QEFIDevicePathMessageI2O(quint32 target);
};

// TODO: Add MSG_InfiniBand  = 0x09

class QEFIDevicePathMessageVendor : public QEFIDevicePathMessage {
protected:
    QUuid m_vendorGuid;
    QByteArray m_vendorData;
public:
    QEFIDevicePathMessageVendor(QUuid vendorGuid, QByteArray vendorData);
};

class QEFIDevicePathMessageMACAddr : public QEFIDevicePathMessage {
protected:
    quint8 m_macAddress[32];
    quint8 m_interfaceType;
public:
    QEFIDevicePathMessageMACAddr(quint8 *macAddress, quint8 interfaceType);
};

class QEFIDevicePathMessageIPv4Addr : public QEFIDevicePathMessage {
protected:
    // TODO: Replace IP address to the Qt ones
    quint8 m_localIPv4Address[4];
    quint8 m_remoteIPv4Address[4];
    quint16 m_localPort;
    quint16 m_remotePort;
    quint16 m_protocol;
    quint8 m_staticIPAddress;
    quint8 m_gateway[4];
    quint8 m_netmask[4];
public:
    QEFIDevicePathMessageIPv4Addr(quint8 *localIPv4Addr, quint8 *remoteIPv4Addr,
        quint16 localPort, quint16 remotePort, quint16 protocol,
        quint8 staticIPAddr, quint8 gateway[4], quint8 netmask[4]);
};

class QEFIDevicePathMessageIPv6Addr : public QEFIDevicePathMessage {
protected:
    // TODO: Replace IP address to the Qt ones
    quint8 m_localIPv6Address[16];
    quint8 m_remoteIPv6Address[16];
    quint16 m_localPort;
    quint16 m_remotePort;
    quint16 m_protocol;
    quint8 m_ipAddressOrigin;
    quint8 m_prefixLength;
    quint8 m_gatewayIPv6Address;
public:
    QEFIDevicePathMessageIPv6Addr(quint8 *localIPv6Addr, quint8 *remoteIPv6Addr,
        quint16 localPort, quint16 remotePort, quint16 protocol,
        quint8 ipAddrOrigin, quint8 prefixLength, quint8 gatewayIPv6Addr);
};

class QEFIDevicePathMessageUART : public QEFIDevicePathMessage {
protected:
    quint32 m_reserved;
    quint64 m_baudRate;
    quint8 m_dataBits;
    quint8 m_parity;
    quint8 m_stopBits;
public:
    QEFIDevicePathMessageUART(quint32 reserved, quint64 baudRate,
        quint8 dataBits, quint8 parity, quint8 stopBits);
};

class QEFIDevicePathMessageUSBClass : public QEFIDevicePathMessage {
protected:
    quint16 m_vendorId;
    quint16 m_productId;
    quint8 m_deviceClass;
    quint8 m_deviceSubclass;
    quint8 m_deviceProtocol;
public:
    QEFIDevicePathMessageUSBClass(quint16 vendorId, quint16 productId,
        quint8 deviceClass, quint8 deviceSubclass,
        quint8 deviceProtocol);
};

class QEFIDevicePathMessageUSBWWID : public QEFIDevicePathMessage {
protected:
    quint16 m_vendorId;
    quint16 m_productId;
    QList<quint16> m_serialNumber;  // TODO: Clarify the SN length
public:
    QEFIDevicePathMessageUSBWWID(quint16 vendorId, quint16 productId,
        quint16 *sn);
};

class QEFIDevicePathMessageLUN : public QEFIDevicePathMessage {
protected:
    quint8 m_lun;
public:
    QEFIDevicePathMessageLUN(quint8 lun);
};

class QEFIDevicePathMessageSATA : public QEFIDevicePathMessage {
protected:
    quint16 m_hbaPort;
    quint16 m_portMultiplierPort;
    quint16 m_lun;
public:
    QEFIDevicePathMessageSATA(quint16 hbaPort,
        quint16 portMultiplierPor, quint8 lun);
};

class QEFIDevicePathMessageISCSI : public QEFIDevicePathMessage {
protected:
    quint16 m_protocol;
    quint16 m_options;
    quint8 m_lun[8];
    quint16 m_tpgt;
    QString m_targetName;
public:
    QEFIDevicePathMessageISCSI(quint16 protocol, quint16 options,
        quint8 *lun, quint16 tpgt, QString targetName);
};

class QEFIDevicePathMessageVLAN : public QEFIDevicePathMessage {
protected:
    quint16 m_vlanID;
public:
    QEFIDevicePathMessageVLAN(quint16 vlanID);
};

class QEFIDevicePathMessageFibreChanEx : public QEFIDevicePathMessage {
protected:
    quint32 m_reserved;
    quint8 m_wwn[8];
    quint8 m_lun[8];
public:
    QEFIDevicePathMessageFibreChanEx(quint32 reserved,
        quint8 *wwn, quint8 *lun);
};

class QEFIDevicePathMessageSASEx : public QEFIDevicePathMessage {
protected:
    quint8 m_sasAddress[8];
    quint8 m_lun[8];
    quint8 m_deviceTopologyInfo;
    quint8 m_driveBayID; /* If EFIDP_SAS_TOPOLOGY_NEXTBYTE set */
    quint16 m_rtp;
public:
    QEFIDevicePathMessageSASEx(quint8 *sasAddress, quint8 *lun,
        quint8 deviceTopologyInfo, quint8 driveBayID, quint16 rtp);
};

class QEFIDevicePathMessageNVME : public QEFIDevicePathMessage {
protected:
    quint32 m_namespaceID;
    quint8 m_ieeeEui64[8];
public:
    QEFIDevicePathMessageNVME(quint32 namespaceID,
        quint8 *ieeeEui64);
};

class QEFIDevicePathMessageURI : public QEFIDevicePathMessage {
protected:
    QUrl m_uri;
public:
    QEFIDevicePathMessageURI(QUrl uri);
};

class QEFIDevicePathMessageUFS : public QEFIDevicePathMessage {
protected:
    quint8 m_targetID;
    quint8 m_lun;
public:
    QEFIDevicePathMessageUFS(quint8 targetID, quint8 lun);
};

class QEFIDevicePathMessageSD : public QEFIDevicePathMessage {
protected:
    quint8 m_slotNumber;
public:
    QEFIDevicePathMessageSD(quint8 slotNumber);
};

class QEFIDevicePathMessageBT : public QEFIDevicePathMessage {
protected:
    quint8 m_address[6];
public:
    QEFIDevicePathMessageBT(quint8 *address);
};

class QEFIDevicePathMessageWiFi : public QEFIDevicePathMessage {
protected:
    QString m_ssid;
public:
    QEFIDevicePathMessageWiFi(QString ssid);
};

class QEFIDevicePathMessageEMMC : public QEFIDevicePathMessage {
protected:
    quint8 m_slotNumber;
public:
    QEFIDevicePathMessageEMMC(quint8 slot);
};

class QEFIDevicePathMessageBTLE : public QEFIDevicePathMessage {
protected:
    quint8 m_address[6];
    quint8 m_addressType;
public:
    QEFIDevicePathMessageBTLE(quint8 *address, quint8 addressType);
};

class QEFIDevicePathMessageDNS : public QEFIDevicePathMessage {
protected:
    // TODO: Add or remove DNS
public:
    QEFIDevicePathMessageDNS();
};

class QEFIDevicePathMessageNVDIMM : public QEFIDevicePathMessage {
protected:
    QUuid m_uuid;
public:
    QEFIDevicePathMessageNVDIMM(QUuid uuid);
};

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
    quint32 partitionNumber() const { return m_partitionNumber; }
    quint64 start() const { return m_start; }
    quint64 size() const { return m_size; }
    // TODO: Get signature according to signature type
    quint8 format() const { return m_format; }
    quint8 signatureType() const { return m_signatureType; }
};

class QEFIDevicePathMediaCDROM : public QEFIDevicePathMedia {
protected:
    quint32 m_bootCatalogEntry;
    quint64 m_partitionRba;
	quint64 m_sectors;
public:
    QEFIDevicePathMediaCDROM(quint32 entry, quint64 partitionRba, quint64 sectors);
    quint32 bootCatalogEntry() const { return m_bootCatalogEntry; }
    quint64 partitionRba() const { return m_partitionRba; }
    quint64 sectors() const { return m_sectors; }
};

class QEFIDevicePathMediaVendor : public QEFIDevicePathMedia {
protected:
    QUuid m_vendorGuid;
    QByteArray m_vendorData;
public:
    QEFIDevicePathMediaVendor(QUuid vendorGuid, QByteArray vendorData);
    QUuid vendorGuid() const { return m_vendorGuid; }
    QByteArray vendorData() const { return m_vendorData; }
};

class QEFIDevicePathMediaFile : public QEFIDevicePathMedia {
protected:
    QString m_name;
public:
    QEFIDevicePathMediaFile(QString name);
    QString name() const;
};

class QEFIDevicePathMediaProtocol : public QEFIDevicePathMedia {
protected:
    QUuid m_protocolGuid;
public:
    QEFIDevicePathMediaProtocol(QUuid protocolGuid);
    QUuid protocolGuid() const { return m_protocolGuid; }
};

class QEFIDevicePathMediaFirmwareFile : public QEFIDevicePathMedia {
protected:
    QByteArray m_piInfo;
public:
    QEFIDevicePathMediaFirmwareFile(QByteArray piInfo);
    QByteArray piInfo() const { return m_piInfo; }
};

class QEFIDevicePathMediaFirmwareVolume : public QEFIDevicePathMedia {
protected:
    QByteArray m_piInfo;
public:
    QEFIDevicePathMediaFirmwareVolume(QByteArray piInfo);
    QByteArray piInfo() const { return m_piInfo; }
};

class QEFIDevicePathMediaRelativeOffset : public QEFIDevicePathMedia {
protected:
    quint32 m_reserved;
    quint64 m_firstByte;
    quint64 m_lastByte;
public:
    QEFIDevicePathMediaRelativeOffset(quint32 reserved,
        quint64 firstByte, quint64 lastByte);
    quint32 reserved() const { return m_reserved; }
    quint64 firstByte() const { return m_firstByte; }
    quint64 lastByte() const { return m_lastByte; }
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
    quint64 startAddress() const { return m_startAddress; }
    quint64 endAddress() const { return m_endAddress; }
    QUuid disktTypeGuid() const { return m_disktTypeGuid; }
    quint16 instanceNumber() const { return m_instanceNumber; }
};

#endif // QEFI_H
