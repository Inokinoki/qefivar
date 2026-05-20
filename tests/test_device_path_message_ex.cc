
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"
#include "../qefi_p.h"

class TestDevicePathMessageEx: public QObject
{
    Q_OBJECT
private slots:
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


void TestDevicePathMessageEx::test_qefi_dp_message_scsi_boundary_values()
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
        QCOMPARE(subP->target(), (quint16)0x0000);
        QCOMPARE(subP->lun(), (quint16)0x0000);
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
        QCOMPARE(subP->target(), (quint16)0xFFFF);
        QCOMPARE(subP->lun(), (quint16)0xFFFF);
    }
}

void TestDevicePathMessageEx::test_qefi_dp_message_iscsi_with_long_target()
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
    QCOMPARE(subP->targetName(), longTarget);
}

void TestDevicePathMessageEx::test_qefi_dp_message_nvme_boundary()
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
        QCOMPARE(subP->namespaceID(), (quint32)0x00000000);
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
        QCOMPARE(subP->namespaceID(), (quint32)0xFFFFFFFF);
    }
}

void TestDevicePathMessageEx::test_qefi_dp_message_uri_http()
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
    QCOMPARE(subP->uri().toString(), QString("http://example.com/path/to/resource"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_uri_https()
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
    QCOMPARE(subP->uri().toString(), QString("https://secure.example.com:443/api"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_uri_ftp()
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
    QCOMPARE(subP->uri().toString(), QString("ftp://ftp.example.com/files/document.pdf"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_uri_tftp()
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
    QCOMPARE(subP->uri().toString(), QString("tftp://192.168.1.1/bootimage"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_uri_file()
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

void TestDevicePathMessageEx::test_qefi_dp_message_wifi_basic()
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
    QCOMPARE(subP->ssid(), QString("MyNetwork"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_wifi_special_chars()
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
    QCOMPARE(subP->ssid(), QString("Test_Network-5G"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_wifi_spaces()
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
    QCOMPARE(subP->ssid(), QString("network with spaces"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_wifi_unicode()
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
    QCOMPARE(subP->ssid(), QString("中文网络"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_wifi_special()
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
    QCOMPARE(subP->ssid(), QString("Test@Network!"));
}

void TestDevicePathMessageEx::test_qefi_dp_message_mac_addr_all_zeros()
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
    QCOMPARE(subP->interfaceType(), (quint8)0x01);
    QEFIDevicePathMessageMACAddress mac = subP->macAddress();
    for (int i = 0; i < 32; i++) {
        QCOMPARE(mac.address[i], (quint8)0);
    }
}

void TestDevicePathMessageEx::test_qefi_dp_message_mac_addr_broadcast()
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
    QCOMPARE(subP->interfaceType(), (quint8)0x01);
    QEFIDevicePathMessageMACAddress mac = subP->macAddress();
    QCOMPARE(mac.address[0], (quint8)0xFF);
    QCOMPARE(mac.address[1], (quint8)0xFF);
    QCOMPARE(mac.address[2], (quint8)0xFF);
    QCOMPARE(mac.address[3], (quint8)0xFF);
    QCOMPARE(mac.address[4], (quint8)0xFF);
    QCOMPARE(mac.address[5], (quint8)0xFF);
}

void TestDevicePathMessageEx::test_qefi_parse_dp_generic_message_nvme()
{
    QEFIDevicePathMessageEUI64 eui {{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}};
    QEFIDevicePathMessageNVME dp(0x00000042, eui.eui);
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(p != nullptr);
    QCOMPARE(p->type(), QEFIDevicePathType::DP_Message);
    QCOMPARE(p->subType(), QEFIDevicePathMessageSubType::MSG_NVME);
    QEFIDevicePathMessageNVME *subP =
        dynamic_cast<QEFIDevicePathMessageNVME *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->namespaceID(), (quint32)0x00000042);
}

QTEST_APPLESS_MAIN(TestDevicePathMessageEx)

#include "test_device_path_message_ex.moc"
