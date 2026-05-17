
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"
#include "../qefi_p.h"

class TestDevicePathMessage: public QObject
{
    Q_OBJECT
private slots:
    void test_qefi_dp_message_atapi();
    void test_qefi_dp_message_scsi();
    void test_qefi_dp_message_fibre_chan();
    void test_qefi_dp_message_1394();
    void test_qefi_dp_message_usb();
    void test_qefi_dp_message_i2o();
    void test_qefi_dp_message_infiniband();
    void test_qefi_dp_message_vendor();
    void test_qefi_dp_message_mac_addr();
    void test_qefi_dp_message_ipv4();
    void test_qefi_dp_message_ipv6();
    void test_qefi_dp_message_uart();
    void test_qefi_dp_message_usb_class();
    void test_qefi_dp_message_usb_wwid();
    void test_qefi_dp_message_lun();
    void test_qefi_dp_message_sata();
    void test_qefi_dp_message_iscsi();
    void test_qefi_dp_message_vlan();
    void test_qefi_dp_message_fibre_chan_ex();
    void test_qefi_dp_message_sas_ex();
    void test_qefi_dp_message_nvme();
    void test_qefi_dp_message_uri();
    void test_qefi_dp_message_ufs();
    void test_qefi_dp_message_sd();
    void test_qefi_dp_message_bt();
    void test_qefi_dp_message_wifi();
    void test_qefi_dp_message_emmc();
    void test_qefi_dp_message_btle();
    void test_qefi_dp_message_dns();
    void test_qefi_dp_message_nvdimm();
    void test_qefi_dp_message_scsi_boundary_values();
    void test_qefi_dp_message_iscsi_with_long_target();
    void test_qefi_dp_message_nvme_boundary();
    void test_qefi_dp_message_uri_http();
    void test_qefi_dp_message_uri_https();
    void test_qefi_dp_message_uri_ftp();
    void test_qefi_dp_message_uri_tftp();
    void test_qefi_dp_message_uri_file();
    void test_qefi_dp_message_wifi_basic();
    void test_qefi_dp_message_wifi_special_chars();
    void test_qefi_dp_message_wifi_spaces();
    void test_qefi_dp_message_wifi_unicode();
    void test_qefi_dp_message_wifi_special();
    void test_qefi_dp_message_mac_addr_all_zeros();
    void test_qefi_dp_message_mac_addr_broadcast();
    void test_qefi_parse_dp_generic_message_nvme();
};


void TestDevicePathMessage::test_qefi_dp_message_atapi()
{
    QEFIDevicePathMessageATAPI dp(/* primary */0x55,
        /* slave */0xAA, /* lun */0x55AA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_ATAPI);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_atapi(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageATAPI *subP =
        dynamic_cast<QEFIDevicePathMessageATAPI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->primary() == dp.primary());
    QVERIFY(subP->slave() == dp.slave());
    QVERIFY(subP->lun() == dp.lun());
}

void TestDevicePathMessage::test_qefi_dp_message_scsi()
{
    QEFIDevicePathMessageSCSI dp(/* target */0xAA55, /* lun */0x55AA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_SCSI);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_scsi(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageSCSI *subP =
        dynamic_cast<QEFIDevicePathMessageSCSI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->target() == dp.target());
    QVERIFY(subP->lun() == dp.lun());
}

void TestDevicePathMessage::test_qefi_dp_message_fibre_chan()
{
    QEFIDevicePathMessageFibreChan dp(/* reserved */0x0,
        /* wwn */0x2233222333114514, /* lun */0x1145142233222333);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_FibreChan);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_fibre_chan(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageFibreChan *subP =
        dynamic_cast<QEFIDevicePathMessageFibreChan *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->reserved() == dp.reserved());
    QVERIFY(subP->wwn() == dp.wwn());
    QVERIFY(subP->lun() == dp.lun());
}

void TestDevicePathMessage::test_qefi_dp_message_1394()
{
    QEFIDevicePathMessage1394 dp(/* reserved */0x0,
        /* guid */0x2233222333114514);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_1394);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_1394(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessage1394 *subP =
        dynamic_cast<QEFIDevicePathMessage1394 *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->reserved() == dp.reserved());
    QVERIFY(subP->guid() == dp.guid());
}

void TestDevicePathMessage::test_qefi_dp_message_usb()
{
    QEFIDevicePathMessageUSB dp(/* parentPort */0x55,
        /* USB Interface */0xAA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_USB);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_usb(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageUSB *subP =
        dynamic_cast<QEFIDevicePathMessageUSB *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->parentPort() == dp.parentPort());
    QVERIFY(subP->usbInterface() == dp.usbInterface());
}

void TestDevicePathMessage::test_qefi_dp_message_i2o()
{
    QEFIDevicePathMessageI2O dp(/* target */0x23114514);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_I2O);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_i2o(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageI2O *subP =
        dynamic_cast<QEFIDevicePathMessageI2O *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->target() == dp.target());
}

void TestDevicePathMessage::test_qefi_dp_message_infiniband()
{
    QEFIDevicePathMessageInfiniBand dp(
        /* resourceFlags */0x23114514,
        /* portGID1 */0x2233222333114514,
        /* portGID2 */0x1145142233222333,
        /* sharedField */0x2211451433222333,
        /* targetPortID */0x2233114514222333,
        /* deviceID */0x2233222114514333);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_InfiniBand);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_infiniband(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageInfiniBand *subP =
        dynamic_cast<QEFIDevicePathMessageInfiniBand *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->resourceFlags() == dp.resourceFlags());
    QVERIFY(subP->portGID1() == dp.portGID1());
    QVERIFY(subP->portGID2() == dp.portGID2());
    QVERIFY(subP->iocGuid() == dp.iocGuid());
    QVERIFY(subP->serviceID() == dp.serviceID());
    QVERIFY(subP->targetPortID() == dp.targetPortID());
    QVERIFY(subP->deviceID() == dp.deviceID());
}

void TestDevicePathMessage::test_qefi_dp_message_vendor()
{
    QEFIDevicePathMessageVendor dp(
        /* uuid */QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"),
        /* data */QByteArray(514, 114));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_Vendor);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_vendor(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageVendor *subP =
        dynamic_cast<QEFIDevicePathMessageVendor *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vendorGuid() == dp.vendorGuid());
    QVERIFY(subP->vendorData() == dp.vendorData());
}

void TestDevicePathMessage::test_qefi_dp_message_mac_addr()
{
    struct QEFIDevicePathMessageMACAddress addr {{
        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99
    }};
    QEFIDevicePathMessageMACAddr dp(addr.address, /* interfaceType */0xAA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_MACAddr);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_mac_addr(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageMACAddr *subP =
        dynamic_cast<QEFIDevicePathMessageMACAddr *>(p.get());
    QVERIFY(subP != nullptr);
    for (int i = 0; i < 32; i++) {
        QVERIFY(subP->macAddress().address[i]
            == dp.macAddress().address[i]);
    }
    QVERIFY(subP->interfaceType() == dp.interfaceType());
}

void TestDevicePathMessage::test_qefi_dp_message_ipv4()
{
    struct QEFIIPv4Address localIPv4Address {{ 192, 168, 0, 1}};
    struct QEFIIPv4Address remoteIPv4Address {{ 192, 168, 0, 100}};
    struct QEFIIPv4Address gateway {{ 192, 168, 0, 254}};
    struct QEFIIPv4Address netmask {{ 255, 255, 255, 0}};
    QEFIDevicePathMessageIPv4Addr dp(localIPv4Address.address,
        remoteIPv4Address.address, /* localPort */ 114, /* remotePort */ 514,
        /* protocol */ 0x55AA, /* staticIPAddress */ 1, gateway.address, netmask.address);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_IPv4);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_ipv4(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageIPv4Addr *subP =
        dynamic_cast<QEFIDevicePathMessageIPv4Addr *>(p.get());
    QVERIFY(subP != nullptr);
    for (int i = 0; i < 4; i++) {
        QVERIFY(subP->localIPv4Address().address[i]
            == dp.localIPv4Address().address[i]);
        QVERIFY(subP->remoteIPv4Address().address[i]
            == dp.remoteIPv4Address().address[i]);
        QVERIFY(subP->gateway().address[i]
            == dp.gateway().address[i]);
        QVERIFY(subP->netmask().address[i]
            == dp.netmask().address[i]);
    }
    QVERIFY(subP->localPort() == dp.localPort());
    QVERIFY(subP->remotePort() == dp.remotePort());
    QVERIFY(subP->protocol() == dp.protocol());
    QVERIFY(subP->staticIPAddress() == dp.staticIPAddress());
}

void TestDevicePathMessage::test_qefi_dp_message_ipv6()
{
    struct QEFIIPv6Address localIPv6Address {{
        0xff, 0xee, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x44, 0x55, 0x66, 0x77, 0x44, 0x55, 0x66, 0x77
    }};
    struct QEFIIPv6Address remoteIPv6Address {{
        0xff, 0xee, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x22, 0x33, 0x44, 0x55, 0x22, 0x33, 0x44, 0x55
    }};
    QEFIDevicePathMessageIPv6Addr dp(localIPv6Address.address,
        remoteIPv6Address.address, /* localPort */ 114, /* remotePort */ 514,
        /* protocol */ 0x55AA, /* ipAddressOrigin */ 0x55,
        /* prefixLength */ 32, /* gatewayIPv6Address */ 0xFF);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_IPv6);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_ipv6(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageIPv6Addr *subP =
        dynamic_cast<QEFIDevicePathMessageIPv6Addr *>(p.get());
    QVERIFY(subP != nullptr);
    for (int i = 0; i < 16; i++) {
        QVERIFY(subP->localIPv6Address().address[i]
            == dp.localIPv6Address().address[i]);
        QVERIFY(subP->remoteIPv6Address().address[i]
            == dp.remoteIPv6Address().address[i]);
    }
    QVERIFY(subP->localPort() == dp.localPort());
    QVERIFY(subP->remotePort() == dp.remotePort());
    QVERIFY(subP->protocol() == dp.protocol());
    QVERIFY(subP->ipAddressOrigin() == dp.ipAddressOrigin());
    QVERIFY(subP->prefixLength() == dp.prefixLength());
    QVERIFY(subP->gatewayIPv6Address() == dp.gatewayIPv6Address());
}

void TestDevicePathMessage::test_qefi_dp_message_uart()
{
    QEFIDevicePathMessageUART dp(/* reserved */ 0x0,
        /* baudRate */ 115200, /* dataBits */ 7,
        /* parity */ 1, /* stopBits */ 1);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_UART);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_uart(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageUART *subP =
        dynamic_cast<QEFIDevicePathMessageUART *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->baudRate() == dp.baudRate());
    QVERIFY(subP->dataBits() == dp.dataBits());
    QVERIFY(subP->parity() == dp.parity());
    QVERIFY(subP->stopBits() == dp.stopBits());
}

void TestDevicePathMessage::test_qefi_dp_message_usb_class()
{
    QEFIDevicePathMessageUSBClass dp(
        /* vendorId */0x55AA, /* productId */0xAA55,
        /* deviceClass */ 0xFF, /* deviceSubclass */ 0xEE,
        /* protocol */ 0xCC);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_USBClass);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_usb_class(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageUSBClass *subP =
        dynamic_cast<QEFIDevicePathMessageUSBClass *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vendorId() == dp.vendorId());
    QVERIFY(subP->productId() == dp.productId());
    QVERIFY(subP->deviceClass() == dp.deviceClass());
    QVERIFY(subP->deviceSubclass() == dp.deviceSubclass());
    QVERIFY(subP->deviceProtocol() == dp.deviceProtocol());
}

void TestDevicePathMessage::test_qefi_dp_message_usb_wwid()
{
    // Test with serial number data
    QList<quint16> serialNumber;
    serialNumber << 0x1234 << 0x5678;
    QEFIDevicePathMessageUSBWWID dp(
        /* vendorId */0x55AA, /* productId */0xAA55, serialNumber);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_USBWWID);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_usb_wwid(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageUSBWWID *subP =
        dynamic_cast<QEFIDevicePathMessageUSBWWID *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vendorId() == dp.vendorId());
    QVERIFY(subP->productId() == dp.productId());
    QVERIFY(subP->serialNumber() == serialNumber);
}

void TestDevicePathMessage::test_qefi_dp_message_lun()
{
    QEFIDevicePathMessageLUN dp(/* lun */0xAA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_LUN);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_lun(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageLUN *subP =
        dynamic_cast<QEFIDevicePathMessageLUN *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->lun() == dp.lun());
}

void TestDevicePathMessage::test_qefi_dp_message_sata()
{
    QEFIDevicePathMessageSATA dp(/* hbaPort */0x55AA,
        /* portMultiplier */0x1, /* lun */0xAA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_SATA);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_sata(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageSATA *subP =
        dynamic_cast<QEFIDevicePathMessageSATA *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->hbaPort() == dp.hbaPort());
    QVERIFY(subP->portMultiplierPort() == dp.portMultiplierPort());
    QVERIFY(subP->lun() == dp.lun());
}

void TestDevicePathMessage::test_qefi_dp_message_iscsi()
{
    QEFIDevicePathMessageLun lun {{
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    }};
    QEFIDevicePathMessageISCSI dp(/* protocol */0x55AA,
        /* options */0xAA55, /* lun */lun.data, /* tpgt */0x55AA,
        QStringLiteral("TestISCSITarget"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_ISCSI);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_iscsi(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageISCSI *subP =
        dynamic_cast<QEFIDevicePathMessageISCSI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->protocol() == dp.protocol());
    QVERIFY(subP->options() == dp.options());
    for (int i = 0; i < 8; i++) {
        QVERIFY(subP->lun().data[i] == dp.lun().data[i]);
    }
    QVERIFY(subP->tpgt() == dp.tpgt());
    QVERIFY(subP->targetName() == dp.targetName());
}

void TestDevicePathMessage::test_qefi_dp_message_vlan()
{
    QEFIDevicePathMessageVLAN dp(/* vlanID */0x55AA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_VLAN);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_vlan(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageVLAN *subP =
        dynamic_cast<QEFIDevicePathMessageVLAN *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vlanID() == dp.vlanID());
}

void TestDevicePathMessage::test_qefi_dp_message_fibre_chan_ex()
{
    QEFIDevicePathMessageLun wwn {{
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
    }};
    QEFIDevicePathMessageLun lun {{
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    }};
    QEFIDevicePathMessageFibreChanEx dp(/* reserved */0x0,
        /* portMultiplier */wwn.data, /* lun */lun.data);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_FibreChanEx);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_fibre_chan_ex(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageFibreChanEx *subP =
        dynamic_cast<QEFIDevicePathMessageFibreChanEx *>(p.get());
    QVERIFY(subP != nullptr);
    for (int i = 0; i < 8; i++) {
        QVERIFY(subP->wwn().data[i] == dp.wwn().data[i]);
        QVERIFY(subP->lun().data[i] == dp.lun().data[i]);
    }
}

void TestDevicePathMessage::test_qefi_dp_message_sas_ex()
{
    QEFIDevicePathMessageSASAddress sasAddress {{
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
    }};
    QEFIDevicePathMessageLun lun {{
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    }};
    QEFIDevicePathMessageSASEx dp(/* address */sasAddress.address,
        /* lun */lun.data, /* deviceTopologyInfo */0x55,
        /* driveBayID */0xAA, /* rtp */0x55AA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_SASEX);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_sas_ex(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageSASEx *subP =
        dynamic_cast<QEFIDevicePathMessageSASEx *>(p.get());
    QVERIFY(subP != nullptr);
    for (int i = 0; i < 8; i++) {
        QVERIFY(subP->lun().data[i] == dp.lun().data[i]);
        QVERIFY(subP->sasAddress().address[i] ==
            dp.sasAddress().address[i]);
    }
    QVERIFY(subP->deviceTopologyInfo() == dp.deviceTopologyInfo());
    QVERIFY(subP->driveBayID() == dp.driveBayID());
    QVERIFY(subP->rtp() == dp.rtp());
}

void TestDevicePathMessage::test_qefi_dp_message_nvme()
{
    QEFIDevicePathMessageEUI64 eui {{
        0x22, 0x33, 0x22, 0x33, 0x22, 0x11, 0x45, 0x14
    }};
    QEFIDevicePathMessageNVME dp(/* namespaceID */0x55,
        /* EUI */eui.eui);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_NVME);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_nvme(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageNVME *subP =
        dynamic_cast<QEFIDevicePathMessageNVME *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->namespaceID() == dp.namespaceID());
    for (int i = 0; i < 8; i++)
        QVERIFY(subP->ieeeEui64().eui[i] == dp.ieeeEui64().eui[i]);
}

void TestDevicePathMessage::test_qefi_dp_message_uri()
{
    QEFIDevicePathMessageURI dp(QUrl("inoki://test"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_URI);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_uri(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageURI *subP =
        dynamic_cast<QEFIDevicePathMessageURI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->uri() == dp.uri());
}

void TestDevicePathMessage::test_qefi_dp_message_ufs()
{
    QEFIDevicePathMessageUFS dp(/* targetID */0x55, /* lun */0xAA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_UFS);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_ufs(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageUFS *subP =
        dynamic_cast<QEFIDevicePathMessageUFS *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->targetID() == dp.targetID());
    QVERIFY(subP->lun() == dp.lun());
}

void TestDevicePathMessage::test_qefi_dp_message_sd()
{
    QEFIDevicePathMessageSD dp(/* slotNumber */0x55);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_SD);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_sd(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageSD *subP =
        dynamic_cast<QEFIDevicePathMessageSD *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->slotNumber() == dp.slotNumber());
}

void TestDevicePathMessage::test_qefi_dp_message_bt()
{
    struct QEFIDevicePathMessageBTAddress address {{
        0xff, 0xee, 0x22, 0x33, 0x44, 0x55
    }};
    QEFIDevicePathMessageBT dp(address.address);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_BT);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_bt(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageBT *subP =
        dynamic_cast<QEFIDevicePathMessageBT *>(p.get());
    QVERIFY(subP != nullptr);
    for (int i = 0; i < 6; i++) {
        QVERIFY(subP->address().address[i]
            == dp.address().address[i]);
    }
}

void TestDevicePathMessage::test_qefi_dp_message_wifi()
{
    QEFIDevicePathMessageWiFi dp(QStringLiteral("Inoki"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_WiFi);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_wifi(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageWiFi *subP =
        dynamic_cast<QEFIDevicePathMessageWiFi *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->ssid() == dp.ssid());
}

void TestDevicePathMessage::test_qefi_dp_message_emmc()
{
    QEFIDevicePathMessageEMMC dp(/* slotNumber */0x55);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_EMMC);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_emmc(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageEMMC *subP =
        dynamic_cast<QEFIDevicePathMessageEMMC *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->slotNumber() == dp.slotNumber());
}

void TestDevicePathMessage::test_qefi_dp_message_btle()
{
    struct QEFIDevicePathMessageBTAddress address {{
        0xff, 0xee, 0x22, 0x33, 0x44, 0x55
    }};
    QEFIDevicePathMessageBTLE dp(address.address, /* addressType */ 0xAA);
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_BTLE);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_btle(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageBTLE *subP =
        dynamic_cast<QEFIDevicePathMessageBTLE *>(p.get());
    QVERIFY(subP != nullptr);
    for (int i = 0; i < 6; i++) {
        QVERIFY(subP->address().address[i]
            == dp.address().address[i]);
    }
    QVERIFY(subP->addressType() == dp.addressType());
}

void TestDevicePathMessage::test_qefi_dp_message_dns()
{
    // DNS has minimal fields currently (isIPv6 flag + TODO addresses)
    // Build raw binary with isIPv6=0 byte after header
    QByteArray raw;
    raw.append((char)QEFIDevicePathType::DP_Message);
    raw.append((char)QEFIDevicePathMessageSubType::MSG_DNS);
    raw.append((char)5); // length LE low
    raw.append((char)0); // length LE high
    raw.append((char)0); // isIPv6 = false

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)raw.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_dns(dp_header, raw.length()));
    QVERIFY(p != nullptr);
    QVERIFY(p->type() == QEFIDevicePathType::DP_Message);
    QVERIFY(p->subType() == QEFIDevicePathMessageSubType::MSG_DNS);
    QEFIDevicePathMessageDNS *subP =
        dynamic_cast<QEFIDevicePathMessageDNS *>(p.get());
    QVERIFY(subP != nullptr);
}

void TestDevicePathMessage::test_qefi_dp_message_nvdimm()
{
    QEFIDevicePathMessageNVDIMM dp(QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Message);
    QVERIFY(dp_header->subtype == QEFIDevicePathMessageSubType::MSG_NVDIMM);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_nvdimm(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMessageNVDIMM *subP =
        dynamic_cast<QEFIDevicePathMessageNVDIMM *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->uuid() == dp.uuid());
}

void TestDevicePathMessage::test_qefi_dp_message_scsi_boundary_values()
{
    {
        QEFIDevicePathMessageSCSI dp(0x0000, 0x0000);
        QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_message_scsi(dp_header, data.length()));
        QEFIDevicePathMessageSCSI *subP =
            dynamic_cast<QEFIDevicePathMessageSCSI *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->target() == 0x0000);
        QVERIFY(subP->lun() == 0x0000);
    }

    {
        QEFIDevicePathMessageSCSI dp(0xFFFF, 0xFFFF);
        QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_message_scsi(dp_header, data.length()));
        QEFIDevicePathMessageSCSI *subP =
            dynamic_cast<QEFIDevicePathMessageSCSI *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->target() == 0xFFFF);
        QVERIFY(subP->lun() == 0xFFFF);
    }
}

void TestDevicePathMessage::test_qefi_dp_message_iscsi_with_long_target()
{
    QString longTarget = "iqn.2024-01.com.example.storage.iscsi.target-long-name-"
                         "for-testing-purposes-with-maximum-allowed-length-"
                         "that-should-still-work-correctly-without-issues";

    QEFIDevicePathMessageLun lun {{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
    }};
    QEFIDevicePathMessageISCSI dp(0x0000, 0x0000, lun.data, 0x0001, longTarget);

    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_iscsi(dp_header, data.length()));
    QEFIDevicePathMessageISCSI *subP =
        dynamic_cast<QEFIDevicePathMessageISCSI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->targetName() == longTarget);
}

void TestDevicePathMessage::test_qefi_dp_message_nvme_boundary()
{
    {
        QEFIDevicePathMessageEUI64 eui {{0x00, 0x01, 0x02, 0x03,
                                               0x04, 0x05, 0x06, 0x07}};
        QEFIDevicePathMessageNVME dp(0x00000000, eui.eui);
        QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_message_nvme(dp_header, data.length()));
        QEFIDevicePathMessageNVME *subP =
            dynamic_cast<QEFIDevicePathMessageNVME *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->namespaceID() == 0x00000000);
    }

    {
        QEFIDevicePathMessageEUI64 eui {{0xFF, 0xEE, 0xDD, 0xCC,
                                               0xBB, 0xAA, 0x99, 0x88}};
        QEFIDevicePathMessageNVME dp(0xFFFFFFFF, eui.eui);
        QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_message_nvme(dp_header, data.length()));
        QEFIDevicePathMessageNVME *subP =
            dynamic_cast<QEFIDevicePathMessageNVME *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->namespaceID() == 0xFFFFFFFF);
    }
}

void TestDevicePathMessage::test_qefi_dp_message_uri_http()
{
    QEFIDevicePathMessageURI dp(QUrl("http://example.com/path/to/resource"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_uri(dp_header, data.length()));
    QEFIDevicePathMessageURI *subP =
        dynamic_cast<QEFIDevicePathMessageURI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->uri().toString() == "http://example.com/path/to/resource");
}

void TestDevicePathMessage::test_qefi_dp_message_uri_https()
{
    QEFIDevicePathMessageURI dp(QUrl("https://secure.example.com:443/api"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_uri(dp_header, data.length()));
    QEFIDevicePathMessageURI *subP =
        dynamic_cast<QEFIDevicePathMessageURI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->uri().toString() == "https://secure.example.com:443/api");
}

void TestDevicePathMessage::test_qefi_dp_message_uri_ftp()
{
    QEFIDevicePathMessageURI dp(QUrl("ftp://ftp.example.com/files/document.pdf"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_uri(dp_header, data.length()));
    QEFIDevicePathMessageURI *subP =
        dynamic_cast<QEFIDevicePathMessageURI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->uri().toString() == "ftp://ftp.example.com/files/document.pdf");
}

void TestDevicePathMessage::test_qefi_dp_message_uri_tftp()
{
    QEFIDevicePathMessageURI dp(QUrl("tftp://192.168.1.1/bootimage"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_uri(dp_header, data.length()));
    QEFIDevicePathMessageURI *subP =
        dynamic_cast<QEFIDevicePathMessageURI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->uri().toString() == "tftp://192.168.1.1/bootimage");
}

void TestDevicePathMessage::test_qefi_dp_message_uri_file()
{
    QEFIDevicePathMessageURI dp(QUrl("file:///local/path/to/file"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_uri(dp_header, data.length()));
    QEFIDevicePathMessageURI *subP =
        dynamic_cast<QEFIDevicePathMessageURI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->uri().toString() == "file:///local/path/to/file");
}

void TestDevicePathMessage::test_qefi_dp_message_wifi_basic()
{
    QEFIDevicePathMessageWiFi dp(QStringLiteral("MyNetwork"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_wifi(dp_header, data.length()));
    QEFIDevicePathMessageWiFi *subP =
        dynamic_cast<QEFIDevicePathMessageWiFi *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->ssid() == "MyNetwork");
}

void TestDevicePathMessage::test_qefi_dp_message_wifi_special_chars()
{
    QEFIDevicePathMessageWiFi dp(QStringLiteral("Test_Network-5G"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_wifi(dp_header, data.length()));
    QEFIDevicePathMessageWiFi *subP =
        dynamic_cast<QEFIDevicePathMessageWiFi *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->ssid() == "Test_Network-5G");
}

void TestDevicePathMessage::test_qefi_dp_message_wifi_spaces()
{
    QEFIDevicePathMessageWiFi dp(QStringLiteral("network with spaces"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_wifi(dp_header, data.length()));
    QEFIDevicePathMessageWiFi *subP =
        dynamic_cast<QEFIDevicePathMessageWiFi *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->ssid() == "network with spaces");
}

void TestDevicePathMessage::test_qefi_dp_message_wifi_unicode()
{
    QEFIDevicePathMessageWiFi dp(QStringLiteral("中文网络"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_wifi(dp_header, data.length()));
    QEFIDevicePathMessageWiFi *subP =
        dynamic_cast<QEFIDevicePathMessageWiFi *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->ssid() == "中文网络");
}

void TestDevicePathMessage::test_qefi_dp_message_wifi_special()
{
    QEFIDevicePathMessageWiFi dp(QStringLiteral("Test@Network!"));
    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_wifi(dp_header, data.length()));
    QEFIDevicePathMessageWiFi *subP =
        dynamic_cast<QEFIDevicePathMessageWiFi *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->ssid() == "Test@Network!");
}

void TestDevicePathMessage::test_qefi_dp_message_mac_addr_all_zeros()
{
    quint8 zeroMac[32] = {0};
    QEFIDevicePathMessageMACAddr dp(zeroMac, 0x01);

    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_mac_addr(dp_header, data.length()));
    QEFIDevicePathMessageMACAddr *subP =
        dynamic_cast<QEFIDevicePathMessageMACAddr *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->interfaceType() == 0x01);
    QEFIDevicePathMessageMACAddress mac = subP->macAddress();
    for (int i = 0; i < 32; i++) {
        QVERIFY(mac.address[i] == 0);
    }
}

void TestDevicePathMessage::test_qefi_dp_message_mac_addr_broadcast()
{
    quint8 broadcastMac[32] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    QEFIDevicePathMessageMACAddr dp(broadcastMac, 0x01);

    QByteArray data = qefi_private_format_message_subtype((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_message_mac_addr(dp_header, data.length()));
    QEFIDevicePathMessageMACAddr *subP =
        dynamic_cast<QEFIDevicePathMessageMACAddr *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->interfaceType() == 0x01);
    QEFIDevicePathMessageMACAddress mac = subP->macAddress();
    QVERIFY(mac.address[0] == 0xFF);
    QVERIFY(mac.address[1] == 0xFF);
    QVERIFY(mac.address[2] == 0xFF);
    QVERIFY(mac.address[3] == 0xFF);
    QVERIFY(mac.address[4] == 0xFF);
    QVERIFY(mac.address[5] == 0xFF);
}

void TestDevicePathMessage::test_qefi_parse_dp_generic_message_nvme()
{
    QEFIDevicePathMessageEUI64 eui {{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}};
    QEFIDevicePathMessageNVME dp(0x00000042, eui.eui);
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(p != nullptr);
    QVERIFY(p->type() == QEFIDevicePathType::DP_Message);
    QVERIFY(p->subType() == QEFIDevicePathMessageSubType::MSG_NVME);
    QEFIDevicePathMessageNVME *subP =
        dynamic_cast<QEFIDevicePathMessageNVME *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->namespaceID() == 0x00000042);
}

QTEST_APPLESS_MAIN(TestDevicePathMessage)

#include "test_device_path_message.moc"
