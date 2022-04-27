
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"

class TestDevicePathHardware: public QObject
{
    Q_OBJECT
private slots:
    void test_qefi_dp_hardware_pci();
    void test_qefi_dp_hardware_pccard();
    void test_qefi_dp_hardware_mmio();
    void test_qefi_dp_hardware_vendor();
    void test_qefi_dp_hardware_controller();
    void test_qefi_dp_hardware_bmc();
};

/* EFI device path header */
#pragma pack(push, 1)
struct qefi_device_path_header {
    quint8 type;
    quint8 subtype;
    quint16 length;
};
#pragma pack(pop)

// Hardware parsing
QEFIDevicePath *qefi_parse_dp_hardware_pci(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_hardware_pccard(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_hardware_mmio(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_hardware_vendor(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_hardware_controller(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_hardware_bmc(
    struct qefi_device_path_header *dp, int dp_size);

// Format Hardware
QByteArray qefi_format_dp_hardware_pci(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_pccard(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_mmio(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_vendor(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_controller(QEFIDevicePath *dp);
QByteArray qefi_format_dp_hardware_bmc(QEFIDevicePath *dp);

void TestDevicePathHardware::test_qefi_dp_hardware_pci()
{
    QEFIDevicePathHardwarePCI dp(/* function */0x55, /* device */0xAA);
    QByteArray data = qefi_format_dp_hardware_pci((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Hardware);
    QVERIFY(dp_header->subtype == QEFIDevicePathHardwareSubType::HW_PCI);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_pci(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathHardwarePCI *subP =
        dynamic_cast<QEFIDevicePathHardwarePCI *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->function() == dp.function());
    QVERIFY(subP->device() == dp.device());
}

void TestDevicePathHardware::test_qefi_dp_hardware_pccard()
{
    QEFIDevicePathHardwarePCCard dp(/* function */0x55);
    QByteArray data = qefi_format_dp_hardware_pccard((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Hardware);
    QVERIFY(dp_header->subtype == QEFIDevicePathHardwareSubType::HW_PCCard);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_pccard(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathHardwarePCCard *subP =
        dynamic_cast<QEFIDevicePathHardwarePCCard *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->function() == dp.function());
}

void TestDevicePathHardware::test_qefi_dp_hardware_mmio()
{
    QEFIDevicePathHardwareMMIO dp(/* memoryType */0x23114514,
        /* startingAddress */0x2233114514,  /* endingAddress */0x2233514114);
    QByteArray data = qefi_format_dp_hardware_mmio((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Hardware);
    QVERIFY(dp_header->subtype == QEFIDevicePathHardwareSubType::HW_MMIO);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_mmio(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathHardwareMMIO *subP =
        dynamic_cast<QEFIDevicePathHardwareMMIO *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->memoryType() == dp.memoryType());
    QVERIFY(subP->startingAddress() == dp.startingAddress());
    QVERIFY(subP->endingAddress() == dp.endingAddress());
}

void TestDevicePathHardware::test_qefi_dp_hardware_vendor()
{
    QEFIDevicePathHardwareVendor dp(
        /* uuid */QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"),
        /* data */QByteArray(514, 114));
    QByteArray data = qefi_format_dp_hardware_vendor((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Hardware);
    QVERIFY(dp_header->subtype == QEFIDevicePathHardwareSubType::HW_Vendor);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_vendor(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathHardwareVendor *subP =
        dynamic_cast<QEFIDevicePathHardwareVendor *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vendorGuid() == dp.vendorGuid());
    QVERIFY(subP->vendorData() == dp.vendorData());
}

void TestDevicePathHardware::test_qefi_dp_hardware_controller()
{
    QEFIDevicePathHardwareController dp(/* controller */0x23114514);
    QByteArray data =
        qefi_format_dp_hardware_controller((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Hardware);
    QVERIFY(dp_header->subtype == QEFIDevicePathHardwareSubType::HW_Controller);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_controller(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathHardwareController *subP =
        dynamic_cast<QEFIDevicePathHardwareController *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->controller() == dp.controller());
}

void TestDevicePathHardware::test_qefi_dp_hardware_bmc()
{
    QEFIDevicePathHardwareBMC dp(/* interfaceType */0x55,
        /* baseAddress */0x2222233333114514);
    QByteArray data = qefi_format_dp_hardware_bmc((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Hardware);
    QVERIFY(dp_header->subtype == QEFIDevicePathHardwareSubType::HW_BMC);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_bmc(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathHardwareBMC *subP =
        dynamic_cast<QEFIDevicePathHardwareBMC *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->interfaceType() == dp.interfaceType());
    QVERIFY(subP->baseAddress() == dp.baseAddress());
}

QTEST_MAIN(TestDevicePathHardware)

#include "test_device_path_hardware.moc"
