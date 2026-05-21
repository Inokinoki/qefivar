
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"
#include "../qefi_p.h"

class TestDevicePathBIOSBoot: public QObject
{
    Q_OBJECT
private slots:
    void test_qefi_dp_biosboot();
    void test_qefi_dp_biosboot_with_description();
    void test_qefi_dp_biosboot_boundary_values();
    void test_qefi_parse_dp_generic_biosboot();
};


void TestDevicePathBIOSBoot::test_qefi_dp_biosboot()
{
    QEFIDevicePathBIOSBoot dp(/* deviceType */0x5555,
        /* status */0xAAAA, /* description */QByteArray());
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_BIOSBoot);
    QCOMPARE(dp_header->subtype, QEFIDevicePathBIOSBootSubType::BIOS_BIOSBoot);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathBIOSBoot *subP =
        dynamic_cast<QEFIDevicePathBIOSBoot *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->deviceType(), dp.deviceType());
    QCOMPARE(subP->status(), dp.status());
    QVERIFY(subP->description().isEmpty());
}

void TestDevicePathBIOSBoot::test_qefi_dp_biosboot_with_description()
{
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
    QCOMPARE(subP->deviceType(), (quint16)0x0101);
    QCOMPARE(subP->status(), (quint16)0x0000);
    QCOMPARE(subP->description(), description);
}

void TestDevicePathBIOSBoot::test_qefi_dp_biosboot_boundary_values()
{
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
        QCOMPARE(subP->deviceType(), (quint16)0x0000);
        QCOMPARE(subP->status(), (quint16)0x0000);
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
        QCOMPARE(subP->deviceType(), (quint16)0xFFFF);
        QCOMPARE(subP->status(), (quint16)0xFFFF);
    }
}

void TestDevicePathBIOSBoot::test_qefi_parse_dp_generic_biosboot()
{
    QEFIDevicePathBIOSBoot dp(0x0101, 0x0000, QByteArray());
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(p != nullptr);
    QCOMPARE(p->type(), QEFIDevicePathType::DP_BIOSBoot);
    QEFIDevicePathBIOSBoot *subP =
        dynamic_cast<QEFIDevicePathBIOSBoot *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->deviceType(), (quint16)0x0101);
}

QTEST_MAIN(TestDevicePathBIOSBoot)

#include "test_device_path_biosboot.moc"
