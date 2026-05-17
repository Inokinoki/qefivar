
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"

class TestDevicePathChain: public QObject
{
    Q_OBJECT
private slots:
    // Multi-device-path chain tests
    void test_chain_acpi_pci_hd_file();
    void test_chain_message_media();
    void test_chain_parse_format_roundtrip();

    // Generic dispatcher edge cases
    void test_qefi_parse_dp_generic_end();

    // Raw binary error handling
    void test_raw_binary_truncated_header();
    void test_raw_binary_truncated_payload();
    void test_raw_binary_invalid_type();
};

/* EFI device path header */
#pragma pack(push, 1)
struct qefi_device_path_header {
    quint8 type;
    quint8 subtype;
    quint16 length;
};
#pragma pack(pop)

QByteArray qefi_format_dp(QEFIDevicePath *dp);
QEFIDevicePath *qefi_parse_dp(struct qefi_device_path_header *dp, int dp_size);

// Multi-device-path chain: ACPI(HID) -> PCI(0,0) -> HD(GPT) -> File
void TestDevicePathChain::test_chain_acpi_pci_hd_file()
{
    QEFIDevicePathACPIHID acpi(0x0a0341d0, 0x00000000);
    QEFIDevicePathHardwarePCI pci(0x00, 0x1F);

    quint8 zeroSignature[16] = {0};
    QEFIDevicePathMediaHD hd(
        /* partitionNumber */ 1,
        /* start */ 2048,
        /* size */ 104857600,
        /* signature */ zeroSignature,
        /* format */ QEFIDevicePathMediaHD::GPT,
        /* signatureType */ QEFIDevicePathMediaHD::GUID);

    QEFIDevicePathMediaFile file(QStringLiteral("\\EFI\\Boot\\bootx64.efi"));

    QByteArray chain;
    chain.append(qefi_format_dp((QEFIDevicePath *)&acpi));
    chain.append(qefi_format_dp((QEFIDevicePath *)&pci));
    chain.append(qefi_format_dp((QEFIDevicePath *)&hd));
    chain.append(qefi_format_dp((QEFIDevicePath *)&file));
    // End of device path
    chain.append((char)0x7F);
    chain.append((char)0xFF);
    chain.append((char)0x04);
    chain.append((char)0x00);

    QList<QSharedPointer<QEFIDevicePath>> parsed;
    quint8 *ptr = (quint8 *)chain.data();
    int remaining = chain.length();
    while (remaining > 4) {
        struct qefi_device_path_header *hdr =
            (struct qefi_device_path_header *)ptr;
        int len = qFromLittleEndian<quint16>(hdr->length);
        if (len < 4 || len > remaining) break;
        if (hdr->type == 0x7F) break;

        QEFIDevicePath *dp = qefi_parse_dp(hdr, len);
        if (dp) parsed.append(QSharedPointer<QEFIDevicePath>(dp));
        ptr += len;
        remaining -= len;
    }

    QVERIFY(parsed.size() == 4);

    QEFIDevicePathACPIHID *p0 = dynamic_cast<QEFIDevicePathACPIHID *>(parsed[0].get());
    QVERIFY(p0 != nullptr);
    QVERIFY(p0->hid() == 0x0a0341d0);

    QEFIDevicePathHardwarePCI *p1 = dynamic_cast<QEFIDevicePathHardwarePCI *>(parsed[1].get());
    QVERIFY(p1 != nullptr);
    QVERIFY(p1->device() == 0x1F);

    QEFIDevicePathMediaHD *p2 = dynamic_cast<QEFIDevicePathMediaHD *>(parsed[2].get());
    QVERIFY(p2 != nullptr);
    QVERIFY(p2->partitionNumber() == 1);

    QEFIDevicePathMediaFile *p3 = dynamic_cast<QEFIDevicePathMediaFile *>(parsed[3].get());
    QVERIFY(p3 != nullptr);
    QVERIFY(p3->name() == "\\EFI\\Boot\\bootx64.efi");
}

// Chain: SATA -> File
void TestDevicePathChain::test_chain_message_media()
{
    QEFIDevicePathMessageSATA sata(0x0000, 0x0000, 0x0000);
    QEFIDevicePathMediaFile file(QStringLiteral("\\EFI\\Microsoft\\Boot\\bootmgfw.efi"));

    QByteArray chain;
    chain.append(qefi_format_dp((QEFIDevicePath *)&sata));
    chain.append(qefi_format_dp((QEFIDevicePath *)&file));
    chain.append((char)0x7F);
    chain.append((char)0xFF);
    chain.append((char)0x04);
    chain.append((char)0x00);

    QList<QSharedPointer<QEFIDevicePath>> parsed;
    quint8 *ptr = (quint8 *)chain.data();
    int remaining = chain.length();
    while (remaining > 4) {
        struct qefi_device_path_header *hdr =
            (struct qefi_device_path_header *)ptr;
        int len = qFromLittleEndian<quint16>(hdr->length);
        if (len < 4 || len > remaining) break;
        if (hdr->type == 0x7F) break;

        QEFIDevicePath *dp = qefi_parse_dp(hdr, len);
        if (dp) parsed.append(QSharedPointer<QEFIDevicePath>(dp));
        ptr += len;
        remaining -= len;
    }

    QVERIFY(parsed.size() == 2);

    QEFIDevicePathMessageSATA *p0 = dynamic_cast<QEFIDevicePathMessageSATA *>(parsed[0].get());
    QVERIFY(p0 != nullptr);
    QVERIFY(p0->hbaPort() == 0x0000);

    QEFIDevicePathMediaFile *p1 = dynamic_cast<QEFIDevicePathMediaFile *>(parsed[1].get());
    QVERIFY(p1 != nullptr);
    QVERIFY(p1->name() == "\\EFI\\Microsoft\\Boot\\bootmgfw.efi");
}

// Full chain round-trip: create -> format -> parse -> re-format -> compare
void TestDevicePathChain::test_chain_parse_format_roundtrip()
{
    QEFIDevicePathACPIHID acpi(0x0a0341d0, 0x00000001);
    QEFIDevicePathHardwarePCI pci(0x02, 0x00);

    QByteArray chainOriginal;
    chainOriginal.append(qefi_format_dp((QEFIDevicePath *)&acpi));
    chainOriginal.append(qefi_format_dp((QEFIDevicePath *)&pci));

    QByteArray chainReformatted;
    quint8 *ptr = (quint8 *)chainOriginal.data();
    int remaining = chainOriginal.length();
    while (remaining > 4) {
        struct qefi_device_path_header *hdr =
            (struct qefi_device_path_header *)ptr;
        int len = qFromLittleEndian<quint16>(hdr->length);
        if (len < 4 || len > remaining) break;

        QEFIDevicePath *dp = qefi_parse_dp(hdr, len);
        if (dp) {
            chainReformatted.append(qefi_format_dp(dp));
            delete dp;
        }
        ptr += len;
        remaining -= len;
    }

    QVERIFY(chainOriginal == chainReformatted);
}

void TestDevicePathChain::test_qefi_parse_dp_generic_end()
{
    QByteArray raw;
    raw.append((char)0x7F);
    raw.append((char)0xFF);
    raw.append((char)0x04);
    raw.append((char)0x00);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)raw.data();
    QEFIDevicePath *p = qefi_parse_dp(dp_header, raw.length());
    QVERIFY(p == nullptr);
}

void TestDevicePathChain::test_raw_binary_truncated_header()
{
    QByteArray raw;
    raw.append((char)0x01);
    raw.append((char)0x01);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)raw.data();
    QEFIDevicePath *p = qefi_parse_dp(dp_header, raw.length());
    QVERIFY(p == nullptr);
}

void TestDevicePathChain::test_raw_binary_truncated_payload()
{
    QByteArray raw;
    raw.append((char)0x01);
    raw.append((char)0x01);
    raw.append((char)0x06);
    raw.append((char)0x00);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)raw.data();
    QEFIDevicePath *p = qefi_parse_dp(dp_header, raw.length());
    QVERIFY(p == nullptr);
}

void TestDevicePathChain::test_raw_binary_invalid_type()
{
    QByteArray raw;
    raw.append((char)0xFE);
    raw.append((char)0x01);
    raw.append((char)0x04);
    raw.append((char)0x00);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)raw.data();
    QEFIDevicePath *p = qefi_parse_dp(dp_header, raw.length());
    QVERIFY(p == nullptr);
}

QTEST_MAIN(TestDevicePathChain)

#include "test_device_path_chain.moc"
