
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi_p.h"

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

    // Edge case tests
    void test_qefi_dp_hardware_pci_boundary_values();
    void test_qefi_dp_hardware_pci_multiple_values();
    void test_qefi_dp_hardware_pccard_boundary();
    void test_qefi_dp_hardware_vendor_empty_data();
    void test_qefi_dp_hardware_vendor_large_data();
    void test_raw_binary_pci();
    void test_qefi_parse_dp_generic_pci();
};

void TestDevicePathHardware::test_qefi_dp_hardware_pci()
{
    QEFIDevicePathHardwarePCI dp(/* function */0x55, /* device */0xAA);
    QByteArray data = qefi_format_dp_hardware_pci((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Hardware);
    QCOMPARE(dp_header->subtype, QEFIDevicePathHardwareSubType::HW_PCI);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_pci(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathHardwarePCI *subP =
        dynamic_cast<QEFIDevicePathHardwarePCI *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->function(), dp.function());
    QCOMPARE(subP->device(), dp.device());
}

void TestDevicePathHardware::test_qefi_dp_hardware_pccard()
{
    QEFIDevicePathHardwarePCCard dp(/* function */0x55);
    QByteArray data = qefi_format_dp_hardware_pccard((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Hardware);
    QCOMPARE(dp_header->subtype, QEFIDevicePathHardwareSubType::HW_PCCard);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_pccard(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathHardwarePCCard *subP =
        dynamic_cast<QEFIDevicePathHardwarePCCard *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->function(), dp.function());
}

void TestDevicePathHardware::test_qefi_dp_hardware_mmio()
{
    QEFIDevicePathHardwareMMIO dp(/* memoryType */0x23114514,
        /* startingAddress */0x2233114514,  /* endingAddress */0x2233514114);
    QByteArray data = qefi_format_dp_hardware_mmio((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Hardware);
    QCOMPARE(dp_header->subtype, QEFIDevicePathHardwareSubType::HW_MMIO);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_mmio(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathHardwareMMIO *subP =
        dynamic_cast<QEFIDevicePathHardwareMMIO *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->memoryType(), dp.memoryType());
    QCOMPARE(subP->startingAddress(), dp.startingAddress());
    QCOMPARE(subP->endingAddress(), dp.endingAddress());
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
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Hardware);
    QCOMPARE(dp_header->subtype, QEFIDevicePathHardwareSubType::HW_Vendor);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_vendor(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathHardwareVendor *subP =
        dynamic_cast<QEFIDevicePathHardwareVendor *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->vendorGuid(), dp.vendorGuid());
    QCOMPARE(subP->vendorData(), dp.vendorData());
}

void TestDevicePathHardware::test_qefi_dp_hardware_controller()
{
    QEFIDevicePathHardwareController dp(/* controller */0x23114514);
    QByteArray data =
        qefi_format_dp_hardware_controller((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Hardware);
    QCOMPARE(dp_header->subtype, QEFIDevicePathHardwareSubType::HW_Controller);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_controller(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathHardwareController *subP =
        dynamic_cast<QEFIDevicePathHardwareController *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->controller(), dp.controller());
}

void TestDevicePathHardware::test_qefi_dp_hardware_bmc()
{
    QEFIDevicePathHardwareBMC dp(/* interfaceType */0x55,
        /* baseAddress */0x2222233333114514);
    QByteArray data = qefi_format_dp_hardware_bmc((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Hardware);
    QCOMPARE(dp_header->subtype, QEFIDevicePathHardwareSubType::HW_BMC);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_hardware_bmc(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathHardwareBMC *subP =
        dynamic_cast<QEFIDevicePathHardwareBMC *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->interfaceType(), dp.interfaceType());
    QCOMPARE(subP->baseAddress(), dp.baseAddress());
}

// Edge case tests
void TestDevicePathHardware::test_qefi_dp_hardware_pci_boundary_values()
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
        QCOMPARE(subP->function(), (quint8)0x00);
        QCOMPARE(subP->device(), (quint8)0x00);
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
        QCOMPARE(subP->function(), (quint8)0xFF);
        QCOMPARE(subP->device(), (quint8)0x1F);
    }
}

void TestDevicePathHardware::test_qefi_dp_hardware_pci_multiple_values()
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
        QCOMPARE(subP->function(), testCases[i][0]);
        QCOMPARE(subP->device(), testCases[i][1]);
    }
}

void TestDevicePathHardware::test_qefi_dp_hardware_pccard_boundary()
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
        QCOMPARE(subP->function(), (quint8)0x00);
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
        QCOMPARE(subP->function(), (quint8)0xFF);
    }
}

void TestDevicePathHardware::test_qefi_dp_hardware_vendor_empty_data()
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
    QCOMPARE(subP->vendorGuid(), dp.vendorGuid());
    QVERIFY(subP->vendorData().isEmpty());
}

void TestDevicePathHardware::test_qefi_dp_hardware_vendor_large_data()
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
    QCOMPARE(subP->vendorGuid(), dp.vendorGuid());
    QCOMPARE(subP->vendorData(), largeData);
}

void TestDevicePathHardware::test_raw_binary_pci()
{
    QByteArray raw;
    raw.append((char)0x01);
    raw.append((char)0x01);
    raw.append((char)0x06);
    raw.append((char)0x00);
    raw.append((char)0x05);
    raw.append((char)0x1C);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)raw.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, raw.length()));
    QVERIFY(p != nullptr);
    QEFIDevicePathHardwarePCI *subP =
        dynamic_cast<QEFIDevicePathHardwarePCI *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->function(), (quint8)0x05);
    QCOMPARE(subP->device(), (quint8)0x1C);

    QByteArray refmt = qefi_format_dp(p.data());
    QCOMPARE(refmt, raw);
}

void TestDevicePathHardware::test_qefi_parse_dp_generic_pci()
{
    QEFIDevicePathHardwarePCI dp(0x03, 0x00);
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(p != nullptr);
    QCOMPARE(p->type(), QEFIDevicePathType::DP_Hardware);
    QCOMPARE(p->subType(), QEFIDevicePathHardwareSubType::HW_PCI);
    QEFIDevicePathHardwarePCI *subP =
        dynamic_cast<QEFIDevicePathHardwarePCI *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->function(), (quint8)0x03);
    QCOMPARE(subP->device(), (quint8)0x00);
}

QTEST_MAIN(TestDevicePathHardware)

#include "test_device_path_hardware.moc"
