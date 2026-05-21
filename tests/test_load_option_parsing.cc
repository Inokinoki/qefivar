#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestLoadOptionParsing: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseTestBootData2();
    void testParseEmptyData();
    void testSettersAndFormat();
    void testParseTruncatedData();
    void testParseCorruptedData();
    void testErrorHandling();
};

void TestLoadOptionParsing::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(loadOption.isVisible());
    QCOMPARE(loadOption.name(), QString(test_boot_name));
    QCOMPARE(loadOption.path(), QString(test_boot_path));
    QCOMPARE(loadOption.devicePathList().size(), 2);

    const auto &devicePathList = loadOption.devicePathList();
    for (const auto &dp : devicePathList) {
        if (dp->type() == QEFIDevicePathType::DP_Media &&
            dp->subType() == QEFIDevicePathMediaSubType::MEDIA_HD) {
            QEFIDevicePathMediaHD *dpMediaHD =
                dynamic_cast<QEFIDevicePathMediaHD *>(dp.get());
            QVERIFY(dpMediaHD != nullptr);
            QCOMPARE(dpMediaHD->signatureType(),
                QEFIDevicePathMediaHD::QEFIDevicePathMediaHDSignatureType::GUID);
            QVERIFY(!dpMediaHD->gptGuid().isNull());
            QCOMPARE(dpMediaHD->gptGuid(),
                QUuid("8632dfd5-910f-4b3d-b250-2c7f17441545"));
            break;
        }
    }
}

void TestLoadOptionParsing::testParseTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(loadOption.isVisible());
    QCOMPARE(loadOption.name(), QString(test_boot_name2));
    QCOMPARE(loadOption.path(), QString(test_boot_path2));
    QCOMPARE(loadOption.devicePathList().size(), 2);
    const auto &devicePathList = loadOption.devicePathList();
    for (const auto &dp : devicePathList) {
        if (dp->type() == QEFIDevicePathType::DP_Media &&
            dp->subType() == QEFIDevicePathMediaSubType::MEDIA_HD) {
            QEFIDevicePathMediaHD *dpMediaHD =
                dynamic_cast<QEFIDevicePathMediaHD *>(dp.get());
            QVERIFY(dpMediaHD != nullptr);
            QCOMPARE(dpMediaHD->signatureType(),
                QEFIDevicePathMediaHD::QEFIDevicePathMediaHDSignatureType::GUID);
            QVERIFY(!dpMediaHD->gptGuid().isNull());
            QCOMPARE(dpMediaHD->gptGuid(),
                QUuid("8632dfd5-910f-4b3d-b250-2c7f17441545"));
            break;
        }
    }
}

void TestLoadOptionParsing::testParseEmptyData()
{
    QByteArray data;
    QEFILoadOption loadOption(data);
    QCOMPARE(loadOption.name().size(), 0);
    QCOMPARE(loadOption.path().size(), 0);
    QCOMPARE(loadOption.devicePathList().size(), 0);
}

void TestLoadOptionParsing::testSettersAndFormat()
{
    // Create an empty QEFILoadOption
    QEFILoadOption loadOption;

    // Set properties
    loadOption.setName("Test Boot");
    loadOption.setIsVisible(true);
    loadOption.setActive(true);
    loadOption.setOptionalData(QByteArray("test"));

    // Add a device path
    loadOption.addDevicePath(QSharedPointer<QEFIDevicePath>(
        new QEFIDevicePathHardwarePCI(0x00, 0x1F)));

    // Verify state before format
    QCOMPARE(loadOption.name(), QString("Test Boot"));
    QVERIFY(loadOption.isVisible());
    QVERIFY(loadOption.isActive());
    QCOMPARE(loadOption.optionalData(), QByteArray("test"));
    QCOMPARE(loadOption.devicePathList().size(), 1);

    // Format and roundtrip
    QByteArray formatted = loadOption.format();
    QVERIFY(formatted.size() > 0);

    // Parse the formatted data into a new object
    QEFILoadOption reparsed(formatted);

    // Verify all fields roundtrip correctly
    QCOMPARE(reparsed.name(), QString("Test Boot"));
    QVERIFY(reparsed.isVisible());
    QVERIFY(reparsed.isActive());
    QCOMPARE(reparsed.optionalData(), QByteArray("test"));
    QCOMPARE(reparsed.devicePathList().size(), 1);

    // Verify device path type roundtrips
    QEFIDevicePathHardwarePCI *dpPCI =
        dynamic_cast<QEFIDevicePathHardwarePCI *>(reparsed.devicePathList()[0].get());
    QVERIFY(dpPCI != nullptr);
    QCOMPARE(dpPCI->function(), (quint8)0x00);
    QCOMPARE(dpPCI->device(), (quint8)0x1F);

    // Test removeDevicePathAt
    reparsed.removeDevicePathAt(0);
    QCOMPARE(reparsed.devicePathList().size(), 0);

    // Test clearDevicePaths
    loadOption.addDevicePath(QSharedPointer<QEFIDevicePath>(
        new QEFIDevicePathHardwarePCI(0x01, 0x02)));
    QCOMPARE(loadOption.devicePathList().size(), 2);
    loadOption.clearDevicePaths();
    QCOMPARE(loadOption.devicePathList().size(), 0);
}

void TestLoadOptionParsing::testParseTruncatedData()
{
    // 2 bytes - too short for even the attribute field
    {
        QByteArray truncated("\x01\x00", 2);
        QEFILoadOption loadOption(truncated);
        // Should handle gracefully without crashing
        // Name and path should be empty or default
    }

    // 5 bytes - has attribute but missing DP length field
    {
        QByteArray truncated((const char *)test_boot_data, 5);
        QEFILoadOption loadOption(truncated);
        // Should handle gracefully - cannot fully parse
    }
}

void TestLoadOptionParsing::testParseCorruptedData()
{
    // Copy valid data and corrupt the DP length field
    QByteArray corrupted((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    // Set DP length to an impossibly large value
    corrupted[4] = (char)0xFF;
    corrupted[5] = (char)0xFF;

    QEFILoadOption loadOption(corrupted);
    // Should not crash; fields may be empty/default or error may be set
    // The important thing is graceful handling
}

void TestLoadOptionParsing::testErrorHandling()
{
    // Valid data should have no error
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(!loadOption.hasError());

    // Empty data should produce an error
    QByteArray emptyData;
    QEFILoadOption emptyOption(emptyData);
    QVERIFY(emptyOption.hasError());
    QVERIFY(!emptyOption.lastError().isEmpty());

    // clearError should reset the error state
    emptyOption.clearError();
    QVERIFY(!emptyOption.hasError());
    QVERIFY(emptyOption.lastError().isEmpty());
}

QTEST_MAIN(TestLoadOptionParsing)

#include "test_load_option_parsing.moc"
