
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"

class TestDevicePathBIOSBoot: public QObject
{
    Q_OBJECT
private slots:
    void test_qefi_dp_biosboot();
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

void TestDevicePathBIOSBoot::test_qefi_dp_biosboot()
{
    QEFIDevicePathBIOSBoot dp(/* deviceType */0x5555,
        /* status */0xAAAA, /* description */QByteArray());
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_BIOSBoot);
    QVERIFY(dp_header->subtype == QEFIDevicePathBIOSBootSubType::BIOS_BIOSBoot);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(!p.isNull());
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathBIOSBoot *subP =
        dynamic_cast<QEFIDevicePathBIOSBoot *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->deviceType() == dp.deviceType());
    QVERIFY(subP->status() == dp.status());
    // TODO: Test description
}

QTEST_MAIN(TestDevicePathBIOSBoot)

#include "test_device_path_biosboot.moc"
