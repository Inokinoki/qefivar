
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"
#include "../qefi_p.h"

class TestDevicePathACPI: public QObject
{
    Q_OBJECT
private slots:
    void test_qefi_dp_acpi_hid();
    void test_qefi_dp_acpi_hidex();
    void test_qefi_dp_acpi_adr();
    void test_qefi_dp_acpi_hid_boundary_values();
    void test_qefi_dp_acpi_hidex_with_strings();
    void test_qefi_dp_acpi_adr_empty_list();
    void test_qefi_dp_acpi_adr_multiple_addresses();
    void test_raw_binary_acpi();
    void test_qefi_parse_dp_generic_acpi_hid();
};


void TestDevicePathACPI::test_qefi_dp_acpi_hid()
{
    QEFIDevicePathACPIHID dp(/* hid */0x55555555, /* uid */0xAAAAAAAA);
    QByteArray data = qefi_format_dp_acpi_hid((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_ACPI);
    QCOMPARE(dp_header->subtype, QEFIDevicePathACPISubType::ACPI_HID);

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_hid(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathACPIHID *subP =
        dynamic_cast<QEFIDevicePathACPIHID *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->hid(), dp.hid());
    QCOMPARE(subP->uid(), dp.uid());
}

void TestDevicePathACPI::test_qefi_dp_acpi_hidex()
{
    QEFIDevicePathACPIHIDEX dp(/* hid */0x55555555,
        /* uid */0xAAAAAAAA, /* cid */0xFFFFFFFF,
        QStringLiteral("HID"), QStringLiteral("UID"),
        QStringLiteral("CID"));
    QByteArray data = qefi_format_dp_acpi_hidex((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_ACPI);
    QCOMPARE(dp_header->subtype, QEFIDevicePathACPISubType::ACPI_HIDEX);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_hidex(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathACPIHIDEX *subP =
        dynamic_cast<QEFIDevicePathACPIHIDEX *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->hid(), dp.hid());
    QCOMPARE(subP->uid(), dp.uid());
    QCOMPARE(subP->cid(), dp.cid());
    // String roundtrip: parser does not yet deserialize HID/UID/CID strings
    QEXPECT_FAIL("", "HIDEX string parsing not yet implemented", Continue);
    QCOMPARE(subP->hidString(), dp.hidString());
    QEXPECT_FAIL("", "HIDEX string parsing not yet implemented", Continue);
    QCOMPARE(subP->uidString(), dp.uidString());
    QEXPECT_FAIL("", "HIDEX string parsing not yet implemented", Continue);
    QCOMPARE(subP->cidString(), dp.cidString());
}

void TestDevicePathACPI::test_qefi_dp_acpi_adr()
{
    QList<quint32> addresses;
    addresses << 0xAAAA5555 << 0x5555AAAA << 0xFFFFFFFF;
    QEFIDevicePathACPIADR dp(addresses);
    QByteArray data = qefi_format_dp_acpi_adr((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_ACPI);
    QCOMPARE(dp_header->subtype, QEFIDevicePathACPISubType::ACPI_ADR);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_adr(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathACPIADR *subP =
        dynamic_cast<QEFIDevicePathACPIADR *>(p.get());
    QVERIFY(subP != nullptr);
    QList<quint32> subPAddresses = subP->addresses();
    QList<quint32> dpAddresses = dp.addresses();
    QCOMPARE(subPAddresses.size(), dpAddresses.size());
    QCOMPARE(subPAddresses[0], dpAddresses[0]);
    QCOMPARE(subPAddresses[1], dpAddresses[1]);
    QCOMPARE(subPAddresses[2], dpAddresses[2]);
}

void TestDevicePathACPI::test_qefi_dp_acpi_hid_boundary_values()
{
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
        QCOMPARE(subP->hid(), (quint32)0x00000000);
        QCOMPARE(subP->uid(), (quint32)0x00000000);
    }

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
        QCOMPARE(subP->hid(), (quint32)0xFFFFFFFF);
        QCOMPARE(subP->uid(), (quint32)0xFFFFFFFF);
    }
}

void TestDevicePathACPI::test_qefi_dp_acpi_hidex_with_strings()
{
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
    QCOMPARE(subP->hid(), dp.hid());
    QCOMPARE(subP->uid(), dp.uid());
    QCOMPARE(subP->cid(), dp.cid());
}

void TestDevicePathACPI::test_qefi_dp_acpi_adr_empty_list()
{
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

void TestDevicePathACPI::test_qefi_dp_acpi_adr_multiple_addresses()
{
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
    QCOMPARE(parsedAddresses.size(), addresses.size());
    for (int i = 0; i < addresses.size(); i++) {
        QCOMPARE(parsedAddresses[i], addresses[i]);
    }
}

void TestDevicePathACPI::test_raw_binary_acpi()
{
    QByteArray raw;
    raw.append((char)0x02);
    raw.append((char)0x01);
    raw.append((char)0x0C);
    raw.append((char)0x00);
    raw.append((char)0xD0); raw.append((char)0x41);
    raw.append((char)0x03); raw.append((char)0x0A);
    raw.append((char)0x01); raw.append((char)0x00);
    raw.append((char)0x00); raw.append((char)0x00);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)raw.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, raw.length()));
    QVERIFY(p != nullptr);
    QEFIDevicePathACPIHID *subP =
        dynamic_cast<QEFIDevicePathACPIHID *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->hid(), (quint32)0x0a0341d0);
    QCOMPARE(subP->uid(), (quint32)0x00000001);
}

void TestDevicePathACPI::test_qefi_parse_dp_generic_acpi_hid()
{
    QEFIDevicePathACPIHID dp(0x0a0341d0, 0x00000001);
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(p != nullptr);
    QCOMPARE(p->type(), QEFIDevicePathType::DP_ACPI);
    QCOMPARE(p->subType(), QEFIDevicePathACPISubType::ACPI_HID);
    QEFIDevicePathACPIHID *subP =
        dynamic_cast<QEFIDevicePathACPIHID *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->hid(), (quint32)0x0a0341d0);
    QCOMPARE(subP->uid(), (quint32)0x00000001);
}

QTEST_MAIN(TestDevicePathACPI)

#include "test_device_path_acpi.moc"
