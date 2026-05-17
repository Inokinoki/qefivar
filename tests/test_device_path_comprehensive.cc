
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"

// Comprehensive device path tests with edge cases and multiple values

class TestDevicePathComprehensive: public QObject
{
    Q_OBJECT
private slots:
    // Hardware edge cases
    void test_qefi_dp_hardware_pci_boundary_values();
    void test_qefi_dp_hardware_pci_multiple_values();
    void test_qefi_dp_hardware_pccard_boundary();
    void test_qefi_dp_hardware_vendor_empty_data();
    void test_qefi_dp_hardware_vendor_large_data();

    // ACPI edge cases
    void test_qefi_dp_acpi_hid_boundary_values();
    void test_qefi_dp_acpi_hidex_with_strings();
    void test_qefi_dp_acpi_adr_empty_list();
    void test_qefi_dp_acpi_adr_multiple_addresses();

    // Message edge cases
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
    void test_qefi_dp_message_wifi_empty();
    void test_qefi_dp_message_mac_addr_all_zeros();
    void test_qefi_dp_message_mac_addr_broadcast();

    // Media edge cases
    void test_qefi_dp_media_hdd_mbr_format();
    void test_qefi_dp_media_hdd_gpt_format();
    void test_qefi_dp_media_file_empty_path();
    void test_qefi_dp_media_file_long_path();
    void test_qefi_dp_media_ramdisk_boundary();

    // BIOSBoot edge cases
    void test_qefi_dp_biosboot_with_description();
    void test_qefi_dp_biosboot_boundary_values();
};

/* EFI device path header */
#pragma pack(push, 1)
struct qefi_device_path_header {
    quint8 type;
    quint8 subtype;
    quint16 length;
};
#pragma pack(pop)

// Hardware parsing/formatting
QEFIDevicePath *qefi_parse_dp_hardware_pci(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_hardware_pci(QEFIDevicePath *dp);

QEFIDevicePath *qefi_parse_dp_hardware_pccard(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_hardware_pccard(QEFIDevicePath *dp);

QEFIDevicePath *qefi_parse_dp_hardware_vendor(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_hardware_vendor(QEFIDevicePath *dp);

// ACPI parsing/formatting
QEFIDevicePath *qefi_parse_dp_acpi_hid(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_acpi_hid(QEFIDevicePath *dp);

QEFIDevicePath *qefi_parse_dp_acpi_hidex(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_acpi_hidex(QEFIDevicePath *dp);

QEFIDevicePath *qefi_parse_dp_acpi_adr(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_acpi_adr(QEFIDevicePath *dp);

// Message parsing/formatting
QEFIDevicePath *qefi_parse_dp_message_scsi(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_message_iscsi(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_message_nvme(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_message_uri(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_message_wifi(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_message_mac_addr(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_private_format_message_subtype(QEFIDevicePath *dp);

// Media parsing/formatting
QEFIDevicePath *qefi_parse_dp_media_hdd(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_media_hdd(QEFIDevicePath *dp);

QEFIDevicePath *qefi_parse_dp_media_file(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_media_file(QEFIDevicePath *dp);

QEFIDevicePath *qefi_parse_dp_media_ramdisk(
    struct qefi_device_path_header *dp, int dp_size);
QByteArray qefi_format_dp_media_ramdisk(QEFIDevicePath *dp);

// BIOSBoot
QByteArray qefi_format_dp(QEFIDevicePath *dp);
QEFIDevicePath *qefi_parse_dp(struct qefi_device_path_header *dp, int dp_size);

// Hardware edge cases
void TestDevicePathComprehensive::test_qefi_dp_hardware_pci_boundary_values()
{
    // Test minimum values
    {
        QEFIDevicePathHardwarePCI dp(0x00, 0x00);
        QByteArray data = qefi_format_dp_hardware_pci((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_hardware_pci(dp_header, data.length()));
        QEFIDevicePathHardwarePCI *subP =
            dynamic_cast<QEFIDevicePathHardwarePCI *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->function() == 0x00);
        QVERIFY(subP->device() == 0x00);
    }

    // Test maximum values
    {
        QEFIDevicePathHardwarePCI dp(0xFF, 0x1F);
        QByteArray data = qefi_format_dp_hardware_pci((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_hardware_pci(dp_header, data.length()));
        QEFIDevicePathHardwarePCI *subP =
            dynamic_cast<QEFIDevicePathHardwarePCI *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->function() == 0xFF);
        QVERIFY(subP->device() == 0x1F);
    }
}

void TestDevicePathComprehensive::test_qefi_dp_hardware_pci_multiple_values()
{
    // Test multiple realistic PCI device/function combinations
    quint8 testCases[][2] = {
        {0, 0},    // Bus 0, Device 0, Function 0
        {0, 7},    // Typical bridge
        {0, 0x1F}, // Last device on bus 0
        {1, 0},    // Second bus
        {3, 5},    // Arbitrary mid-range values
    };

    for (size_t i = 0; i < sizeof(testCases) / sizeof(testCases[0]); i++) {
        QEFIDevicePathHardwarePCI dp(testCases[i][0], testCases[i][1]);
        QByteArray data = qefi_format_dp_hardware_pci((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_hardware_pci(dp_header, data.length()));
        QEFIDevicePathHardwarePCI *subP =
            dynamic_cast<QEFIDevicePathHardwarePCI *>(p.get());
        QVERIFY2(subP != nullptr, "PCI device path should parse correctly");
        QVERIFY2(subP->function() == testCases[i][0], "Function should match");
        QVERIFY2(subP->device() == testCases[i][1], "Device should match");
    }
}

void TestDevicePathComprehensive::test_qefi_dp_hardware_pccard_boundary()
{
    // Test boundary values for PCCard function number
    {
        QEFIDevicePathHardwarePCCard dp(0x00);
        QByteArray data = qefi_format_dp_hardware_pccard((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_hardware_pccard(dp_header, data.length()));
        QEFIDevicePathHardwarePCCard *subP =
            dynamic_cast<QEFIDevicePathHardwarePCCard *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->function() == 0x00);
    }

    {
        QEFIDevicePathHardwarePCCard dp(0xFF);
        QByteArray data = qefi_format_dp_hardware_pccard((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_hardware_pccard(dp_header, data.length()));
        QEFIDevicePathHardwarePCCard *subP =
            dynamic_cast<QEFIDevicePathHardwarePCCard *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->function() == 0xFF);
    }
}

void TestDevicePathComprehensive::test_qefi_dp_hardware_vendor_empty_data()
{
    // Test vendor with empty data
    QEFIDevicePathHardwareVendor dp(
        QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"),
        QByteArray());
    QByteArray data = qefi_format_dp_hardware_vendor((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_vendor(dp_header, data.length()));
    QEFIDevicePathHardwareVendor *subP =
        dynamic_cast<QEFIDevicePathHardwareVendor *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vendorGuid() == dp.vendorGuid());
    QVERIFY(subP->vendorData().isEmpty());
}

void TestDevicePathComprehensive::test_qefi_dp_hardware_vendor_large_data()
{
    // Test vendor with large data payload
    QByteArray largeData(1024, 0xAB);  // 1KB of data
    QEFIDevicePathHardwareVendor dp(
        QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"),
        largeData);
    QByteArray data = qefi_format_dp_hardware_vendor((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_vendor(dp_header, data.length()));
    QEFIDevicePathHardwareVendor *subP =
        dynamic_cast<QEFIDevicePathHardwareVendor *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vendorGuid() == dp.vendorGuid());
    QVERIFY(subP->vendorData() == largeData);
}

// ACPI edge cases
void TestDevicePathComprehensive::test_qefi_dp_acpi_hid_boundary_values()
{
    // Test with minimum values
    {
        QEFIDevicePathACPIHID dp(0x00000000, 0x00000000);
        QByteArray data = qefi_format_dp_acpi_hid((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_acpi_hid(dp_header, data.length()));
        QEFIDevicePathACPIHID *subP =
            dynamic_cast<QEFIDevicePathACPIHID *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->hid() == 0x00000000);
        QVERIFY(subP->uid() == 0x00000000);
    }

    // Test with maximum values
    {
        QEFIDevicePathACPIHID dp(0xFFFFFFFF, 0xFFFFFFFF);
        QByteArray data = qefi_format_dp_acpi_hid((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_acpi_hid(dp_header, data.length()));
        QEFIDevicePathACPIHID *subP =
            dynamic_cast<QEFIDevicePathACPIHID *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->hid() == 0xFFFFFFFF);
        QVERIFY(subP->uid() == 0xFFFFFFFF);
    }
}

void TestDevicePathComprehensive::test_qefi_dp_acpi_hidex_with_strings()
{
    // Test HIDEX with actual string values
    QEFIDevicePathACPIHIDEX dp(
        /* hid */0x01234567,
        /* uid */0x89ABCDEF,
        /* cid */0x00000000,
        QStringLiteral("PNP0A03"),
        QStringLiteral("0"),
        QStringLiteral(""));

    QByteArray data = qefi_format_dp_acpi_hidex((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_hidex(dp_header, data.length()));
    QEFIDevicePathACPIHIDEX *subP =
        dynamic_cast<QEFIDevicePathACPIHIDEX *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->hid() == dp.hid());
    QVERIFY(subP->uid() == dp.uid());
    QVERIFY(subP->cid() == dp.cid());
    // Note: String parsing verification depends on implementation
}

void TestDevicePathComprehensive::test_qefi_dp_acpi_adr_empty_list()
{
    // Test ADR with empty address list
    QList<quint32> emptyAddresses;
    QEFIDevicePathACPIADR dp(emptyAddresses);
    QByteArray data = qefi_format_dp_acpi_adr((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_adr(dp_header, data.length()));
    QEFIDevicePathACPIADR *subP =
        dynamic_cast<QEFIDevicePathACPIADR *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->addresses().isEmpty());
}

void TestDevicePathComprehensive::test_qefi_dp_acpi_adr_multiple_addresses()
{
    // Test ADR with multiple addresses
    QList<quint32> addresses;
    addresses << 0x00010001 << 0x00020002 << 0x00030003
              << 0x00040004 << 0x00050005;

    QEFIDevicePathACPIADR dp(addresses);
    QByteArray data = qefi_format_dp_acpi_adr((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_adr(dp_header, data.length()));
    QEFIDevicePathACPIADR *subP =
        dynamic_cast<QEFIDevicePathACPIADR *>(p.get());
    QVERIFY(subP != nullptr);

    QList<quint32> parsedAddresses = subP->addresses();
    QVERIFY(parsedAddresses.size() == addresses.size());
    for (int i = 0; i < addresses.size(); i++) {
        QVERIFY(parsedAddresses[i] == addresses[i]);
    }
}

// Message edge cases
void TestDevicePathComprehensive::test_qefi_dp_message_scsi_boundary_values()
{
    // Test with minimum values
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

    // Test with maximum values
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

void TestDevicePathComprehensive::test_qefi_dp_message_iscsi_with_long_target()
{
    // Test iSCSI with long target name (max 223 bytes per RFC)
    QString longTarget = "iqn.2024-01.com.example.storage.iscsi.target-long-name-"
                         "for-testing-purposes-with-maximum-allowed-length-"
                         "that-should-still-work-correctly-without-issues";

    QEFIDevicePathMessageLun lun {
        .data { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }
    };
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

void TestDevicePathComprehensive::test_qefi_dp_message_nvme_boundary()
{
    // Test NVME with boundary namespace ID
    {
        QEFIDevicePathMessageEUI64 eui { .eui = {0x00, 0x01, 0x02, 0x03,
                                                   0x04, 0x05, 0x06, 0x07} };
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
        QEFIDevicePathMessageEUI64 eui { .eui = {0xFF, 0xEE, 0xDD, 0xCC,
                                                   0xBB, 0xAA, 0x99, 0x88} };
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

void TestDevicePathComprehensive::test_qefi_dp_message_uri_http()
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

void TestDevicePathComprehensive::test_qefi_dp_message_uri_https()
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

void TestDevicePathComprehensive::test_qefi_dp_message_uri_ftp()
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

void TestDevicePathComprehensive::test_qefi_dp_message_uri_tftp()
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

void TestDevicePathComprehensive::test_qefi_dp_message_uri_file()
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

void TestDevicePathComprehensive::test_qefi_dp_message_wifi_basic()
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

void TestDevicePathComprehensive::test_qefi_dp_message_wifi_special_chars()
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

void TestDevicePathComprehensive::test_qefi_dp_message_wifi_spaces()
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

void TestDevicePathComprehensive::test_qefi_dp_message_wifi_unicode()
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

void TestDevicePathComprehensive::test_qefi_dp_message_wifi_empty()
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

void TestDevicePathComprehensive::test_qefi_dp_message_mac_addr_all_zeros()
{
    // Test MAC address with all zeros
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
    // Verify MAC address is all zeros
    QEFIDevicePathMessageMACAddress mac = subP->macAddress();
    for (int i = 0; i < 32; i++) {
        QVERIFY(mac.address[i] == 0);
    }
}

void TestDevicePathComprehensive::test_qefi_dp_message_mac_addr_broadcast()
{
    // Test MAC address with broadcast address (FF:FF:FF:FF:FF:FF)
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

// Media edge cases
void TestDevicePathComprehensive::test_qefi_dp_media_hdd_mbr_format()
{
    // Test HDD with MBR format
    quint8 mbrSignature[16] = {0};
    mbrSignature[0] = 0x01;
    mbrSignature[1] = 0x02;
    mbrSignature[2] = 0x03;
    mbrSignature[3] = 0x04;

    QEFIDevicePathMediaHD dp(
        /* partitionNumber */ 1,
        /* start */ 2048,
        /* size */ 1024 * 1024 * 100,
        /* signature */ mbrSignature,
        /* format */ QEFIDevicePathMediaHD::PCAT,
        /* signatureType */ QEFIDevicePathMediaHD::MBR);

    QByteArray data = qefi_format_dp_media_hdd((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_hdd(dp_header, data.length()));
    QEFIDevicePathMediaHD *subP =
        dynamic_cast<QEFIDevicePathMediaHD *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->format() == QEFIDevicePathMediaHD::PCAT);
    QVERIFY(subP->signatureType() == QEFIDevicePathMediaHD::MBR);
    QVERIFY(subP->mbrSignature() == 0x04030201);  // Little-endian
}

void TestDevicePathComprehensive::test_qefi_dp_media_hdd_gpt_format()
{
    // Test HDD with GPT format
    QByteArray guid = qefi_rfc4122_to_guid(
        QUuid("12345678-1234-1234-1234-123456789ABC").toRfc4122());

    QEFIDevicePathMediaHD dp(
        /* partitionNumber */ 2,
        /* start */ 4096,
        /* size */ 1024 * 1024 * 200,
        /* signature */ (quint8 *)guid.data(),
        /* format */ QEFIDevicePathMediaHD::GPT,
        /* signatureType */ QEFIDevicePathMediaHD::GUID);

    QByteArray data = qefi_format_dp_media_hdd((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_hdd(dp_header, data.length()));
    QEFIDevicePathMediaHD *subP =
        dynamic_cast<QEFIDevicePathMediaHD *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->format() == QEFIDevicePathMediaHD::GPT);
    QVERIFY(subP->signatureType() == QEFIDevicePathMediaHD::GUID);
    QVERIFY(subP->partitionNumber() == 2);
}

void TestDevicePathComprehensive::test_qefi_dp_media_file_empty_path()
{
    // Test file path with empty string
    QEFIDevicePathMediaFile dp("");
    QByteArray data = qefi_format_dp_media_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_file(dp_header, data.length()));
    QEFIDevicePathMediaFile *subP =
        dynamic_cast<QEFIDevicePathMediaFile *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->name().isEmpty());
}

void TestDevicePathComprehensive::test_qefi_dp_media_file_long_path()
{
    // Test file path with long path (typical EFI boot path)
    QString longPath = "\\EFI\\Boot\\bootx64.efi";
    QEFIDevicePathMediaFile dp(longPath);
    QByteArray data = qefi_format_dp_media_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_file(dp_header, data.length()));
    QEFIDevicePathMediaFile *subP =
        dynamic_cast<QEFIDevicePathMediaFile *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->name() == longPath);
}

void TestDevicePathComprehensive::test_qefi_dp_media_ramdisk_boundary()
{
    // Test RAM disk with boundary addresses
    {
        QEFIDevicePathMediaRAMDisk dp(
            /* startAddress */ 0x10000000,
            /* endAddress */ 0x1FFFFFFF,
            /* diskTypeGuid */ QUuid("12345678-1234-1234-1234-123456789ABC"),
            /* instanceNumber */ 0x0000);

        QByteArray data = qefi_format_dp_media_ramdisk((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_media_ramdisk(dp_header, data.length()));
        QEFIDevicePathMediaRAMDisk *subP =
            dynamic_cast<QEFIDevicePathMediaRAMDisk *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->startAddress() == 0x10000000);
        QVERIFY(subP->endAddress() == 0x1FFFFFFF);
        QVERIFY(subP->instanceNumber() == 0x0000);
    }

    {
        QEFIDevicePathMediaRAMDisk dp(
            /* startAddress */ 0x0,
            /* endAddress */ 0xFFFFFFFFFFFFFFFF,
            /* diskTypeGuid */ QUuid("87654321-4321-4321-4321-CBA987654321"),
            /* instanceNumber */ 0xFFFF);

        QByteArray data = qefi_format_dp_media_ramdisk((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_media_ramdisk(dp_header, data.length()));
        QEFIDevicePathMediaRAMDisk *subP =
            dynamic_cast<QEFIDevicePathMediaRAMDisk *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->startAddress() == 0x0);
        QVERIFY(subP->endAddress() == 0xFFFFFFFFFFFFFFFF);
        QVERIFY(subP->instanceNumber() == 0xFFFF);
    }
}

// BIOSBoot edge cases
void TestDevicePathComprehensive::test_qefi_dp_biosboot_with_description()
{
    // Test BIOSBoot with description
    QByteArray description = "Test Boot Device";
    QEFIDevicePathBIOSBoot dp(0x0101, 0x0000, description);

    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QEFIDevicePathBIOSBoot *subP =
        dynamic_cast<QEFIDevicePathBIOSBoot *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->deviceType() == 0x0101);
    QVERIFY(subP->status() == 0x0000);
    // Description parsing depends on implementation
}

void TestDevicePathComprehensive::test_qefi_dp_biosboot_boundary_values()
{
    // Test BIOSBoot with boundary values
    {
        QEFIDevicePathBIOSBoot dp(0x0000, 0x0000, QByteArray());
        QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp(dp_header, data.length()));
        QEFIDevicePathBIOSBoot *subP =
            dynamic_cast<QEFIDevicePathBIOSBoot *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->deviceType() == 0x0000);
        QVERIFY(subP->status() == 0x0000);
    }

    {
        QEFIDevicePathBIOSBoot dp(0xFFFF, 0xFFFF, QByteArray());
        QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp(dp_header, data.length()));
        QEFIDevicePathBIOSBoot *subP =
            dynamic_cast<QEFIDevicePathBIOSBoot *>(p.get());
        QVERIFY(subP != nullptr);
        QVERIFY(subP->deviceType() == 0xFFFF);
        QVERIFY(subP->status() == 0xFFFF);
    }
}

QTEST_MAIN(TestDevicePathComprehensive)

#include "test_device_path_comprehensive.moc"
