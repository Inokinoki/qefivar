
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"

class TestDevicePathACPI: public QObject
{
    Q_OBJECT
private slots:
    void test_qefi_dp_acpi_hid();
    void test_qefi_dp_acpi_hidex();
    void test_qefi_dp_acpi_adr();
};

/* EFI device path header */
#pragma pack(push, 1)
struct qefi_device_path_header {
    quint8 type;
    quint8 subtype;
    quint16 length;
};
#pragma pack(pop)

// ACPI parsing
QEFIDevicePath *qefi_parse_dp_acpi_hid(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_acpi_hidex(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_acpi_adr(
    struct qefi_device_path_header *dp, int dp_size);

// Format ACPI
QByteArray qefi_format_dp_acpi_hid(QEFIDevicePath *dp);
QByteArray qefi_format_dp_acpi_hidex(QEFIDevicePath *dp);
QByteArray qefi_format_dp_acpi_adr(QEFIDevicePath *dp);


void TestDevicePathACPI::test_qefi_dp_acpi_hid()
{
    QEFIDevicePathACPIHID dp(/* hid */0x55555555, /* uid */0xAAAAAAAA);
    QByteArray data = qefi_format_dp_acpi_hid((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_ACPI);
    QVERIFY(dp_header->subtype == QEFIDevicePathACPISubType::ACPI_HID);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1)))
        == dp.hid());
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1) +
        sizeof(quint32))) == dp.uid());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_hid(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathACPIHID *subP =
        dynamic_cast<QEFIDevicePathACPIHID *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->hid() == dp.hid());
    QVERIFY(subP->uid() == dp.uid());
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
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_ACPI);
    QVERIFY(dp_header->subtype == QEFIDevicePathACPISubType::ACPI_HIDEX);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1)))
        == dp.hid());
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1) +
        sizeof(quint32))) == dp.uid());
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1) +
        sizeof(quint32) + sizeof(quint32))) == dp.cid());
    // TODO: Test strings format

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_hidex(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathACPIHIDEX *subP =
        dynamic_cast<QEFIDevicePathACPIHIDEX *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->hid() == dp.hid());
    QVERIFY(subP->uid() == dp.uid());
    QVERIFY(subP->cid() == dp.cid());
    // TODO: Test strings parsing
    /*
    QVERIFY(subP->hidString() == dp.hidString());
    QVERIFY(subP->uidString() == dp.uidString());
    QVERIFY(subP->cidString() == dp.cidString());
    */
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
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_ACPI);
    QVERIFY(dp_header->subtype == QEFIDevicePathACPISubType::ACPI_ADR);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1)))
        == addresses[0]);
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1) +
        sizeof(quint32))) == addresses[1]);
    QVERIFY(qFromLittleEndian<quint32>(*(quint32 *)((quint8 *)(dp_header + 1) +
        sizeof(quint32) + sizeof(quint32))) == addresses[2]);

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_acpi_adr(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathACPIADR *subP =
        dynamic_cast<QEFIDevicePathACPIADR *>(p.get());
    QVERIFY(subP != nullptr);
    QList<quint32> subPAddresses = subP->addresses();
    QList<quint32> dpAddresses = dp.addresses();
    QVERIFY(subPAddresses.size() == dpAddresses.size());
    QVERIFY(subPAddresses[0] == dpAddresses[0]);
    QVERIFY(subPAddresses[1] == dpAddresses[1]);
    QVERIFY(subPAddresses[2] == dpAddresses[2]);
}

QTEST_MAIN(TestDevicePathACPI)

#include "test_device_path_acpi.moc"
