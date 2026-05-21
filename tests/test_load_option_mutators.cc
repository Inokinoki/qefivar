#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestLoadOptionMutators: public QObject
{
    Q_OBJECT
private slots:
    void testSetName();
    void testSetIsVisible();
    void testSetOptionalData();
    void testSetActive();
    void testSetHidden();
    void testSetForceReconnect();
    void testSetCategory();
    void testCategoryTypeWidth();
    void testCategoryRoundtrip();
    void testCategoryBoundaryValues();
    void testCategoryMasking();
    void testAttributesFlags();
    void testDefaultConstructor();
    void testCreateFromScratch();
    void testHasError();
    void testLastError();
    void testIsValid();
    void testTruncatedData();
    void testCorruptedHeader();
};

void TestLoadOptionMutators::testSetName()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);

    QCOMPARE(loadOption.name(), QString(test_boot_name));

    loadOption.setName("New Boot Name");
    QCOMPARE(loadOption.name(), QString("New Boot Name"));

    // Verify format reflects the change
    QByteArray formatted = loadOption.format();
    QEFILoadOption reparsed(formatted);
    QCOMPARE(reparsed.name(), QString("New Boot Name"));
}

void TestLoadOptionMutators::testSetIsVisible()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);

    QVERIFY(loadOption.isVisible());

    loadOption.setIsVisible(false);
    QVERIFY(!loadOption.isVisible());

    loadOption.setIsVisible(true);
    QVERIFY(loadOption.isVisible());

    // Verify format reflects the change
    QByteArray formatted = loadOption.format();
    QEFILoadOption reparsed(formatted);
    QVERIFY(reparsed.isVisible());
}

void TestLoadOptionMutators::testSetOptionalData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);

    QVERIFY(loadOption.optionalData().isEmpty());

    QByteArray newOptionalData("WINDOWS");
    loadOption.setOptionalData(newOptionalData);
    QCOMPARE(loadOption.optionalData(), newOptionalData);

    // Clear optional data
    loadOption.setOptionalData(QByteArray());
    QVERIFY(loadOption.optionalData().isEmpty());
}

void TestLoadOptionMutators::testSetActive()
{
    QEFILoadOption loadOption;

    // Default: not active
    QVERIFY(!loadOption.isActive());

    loadOption.setActive(true);
    QVERIFY(loadOption.isActive());
    QCOMPARE(loadOption.attributes() & QEFI_LOAD_OPTION_ACTIVE,
             (quint32)QEFI_LOAD_OPTION_ACTIVE);

    loadOption.setActive(false);
    QVERIFY(!loadOption.isActive());
    QCOMPARE(loadOption.attributes() & QEFI_LOAD_OPTION_ACTIVE, (quint32)0);

    // Roundtrip through format/parse
    loadOption.setActive(true);
    loadOption.setName("ActiveTest");
    QByteArray formatted = loadOption.format();
    QEFILoadOption reparsed(formatted);
    QVERIFY(reparsed.isActive());
}

void TestLoadOptionMutators::testSetHidden()
{
    QEFILoadOption loadOption;

    // Default: not hidden
    QVERIFY(!loadOption.isHidden());

    loadOption.setHidden(true);
    QVERIFY(loadOption.isHidden());
    QCOMPARE(loadOption.attributes() & QEFI_LOAD_OPTION_HIDDEN,
             (quint32)QEFI_LOAD_OPTION_HIDDEN);

    loadOption.setHidden(false);
    QVERIFY(!loadOption.isHidden());
    QCOMPARE(loadOption.attributes() & QEFI_LOAD_OPTION_HIDDEN, (quint32)0);

    // Roundtrip through format/parse
    loadOption.setHidden(true);
    loadOption.setName("HiddenTest");
    QByteArray formatted = loadOption.format();
    QEFILoadOption reparsed(formatted);
    QVERIFY(reparsed.isHidden());
}

void TestLoadOptionMutators::testSetForceReconnect()
{
    QEFILoadOption loadOption;

    // Default: not force reconnect
    QVERIFY(!loadOption.isForceReconnect());

    loadOption.setForceReconnect(true);
    QVERIFY(loadOption.isForceReconnect());
    QCOMPARE(loadOption.attributes() & QEFI_LOAD_OPTION_FORCE_RECONNECT,
             (quint32)QEFI_LOAD_OPTION_FORCE_RECONNECT);

    loadOption.setForceReconnect(false);
    QVERIFY(!loadOption.isForceReconnect());
    QCOMPARE(loadOption.attributes() & QEFI_LOAD_OPTION_FORCE_RECONNECT, (quint32)0);
}

void TestLoadOptionMutators::testSetCategory()
{
    QEFILoadOption loadOption;

    // Default category: boot (0x00)
    QCOMPARE(loadOption.category(), (quint8)QEFI_LOAD_OPTION_CATEGORY_BOOT);

    loadOption.setCategory(QEFI_LOAD_OPTION_CATEGORY_APP >> 8);
    QCOMPARE(loadOption.category(), (quint8)(QEFI_LOAD_OPTION_CATEGORY_APP >> 8));
    QCOMPARE(loadOption.attributes() & QEFI_LOAD_OPTION_CATEGORY_MASK,
             (quint32)QEFI_LOAD_OPTION_CATEGORY_APP);

    // Set back to boot
    loadOption.setCategory(0);
    QCOMPARE(loadOption.category(), (quint8)0);
}

void TestLoadOptionMutators::testCategoryTypeWidth()
{
    // Category is quint8 (8-bit), stored in bits 8-12 of attributes (5-bit field)
    // Verify that the type is exactly quint8 and not wider
    QEFILoadOption loadOption;

    // Setting a value that fits in 5 bits should work
    loadOption.setCategory(0x1F);  // max 5-bit value
    QCOMPARE(loadOption.category(), (quint8)0x1F);

    // Values above 5 bits should be masked (only low 5 bits used)
    loadOption.setCategory(0xFF);
    QCOMPARE(loadOption.category(), (quint8)0x1F);  // 0xFF & 0x1F = 0x1F

    loadOption.setCategory(0x20);  // bit 5 set, should be masked out
    QCOMPARE(loadOption.category(), (quint8)0x00);  // 0x20 & 0x1F = 0x00
}

void TestLoadOptionMutators::testCategoryRoundtrip()
{
    // Category should survive format/parse roundtrip
    QEFILoadOption loadOption;
    loadOption.setName("CategoryTest");
    loadOption.setIsVisible(true);

    // Test BOOT category roundtrip
    loadOption.setCategory(QEFI_LOAD_OPTION_CATEGORY_BOOT >> 8);
    QByteArray bootFormatted = loadOption.format();
    QEFILoadOption bootReparsed(bootFormatted);
    QCOMPARE(bootReparsed.category(), (quint8)(QEFI_LOAD_OPTION_CATEGORY_BOOT >> 8));

    // Test APP category roundtrip
    loadOption.setCategory(QEFI_LOAD_OPTION_CATEGORY_APP >> 8);
    QByteArray appFormatted = loadOption.format();
    QEFILoadOption appReparsed(appFormatted);
    QCOMPARE(appReparsed.category(), (quint8)(QEFI_LOAD_OPTION_CATEGORY_APP >> 8));
}

void TestLoadOptionMutators::testCategoryBoundaryValues()
{
    QEFILoadOption loadOption;

    // Min value
    loadOption.setCategory(0);
    QCOMPARE(loadOption.category(), (quint8)0);

    // Max valid value (all 5 bits set)
    loadOption.setCategory(0x1F);
    QCOMPARE(loadOption.category(), (quint8)0x1F);

    // Each bit individually
    for (int i = 0; i < 5; i++) {
        loadOption.setCategory(1 << i);
        QCOMPARE(loadOption.category(), (quint8)(1 << i));
    }
}

void TestLoadOptionMutators::testCategoryMasking()
{
    // Category bits must not interfere with other attribute bits
    QEFILoadOption loadOption;

    loadOption.setActive(true);
    loadOption.setHidden(true);
    loadOption.setForceReconnect(true);
    loadOption.setCategory(QEFI_LOAD_OPTION_CATEGORY_APP >> 8);

    // All flags should still be set correctly
    QVERIFY(loadOption.isActive());
    QVERIFY(loadOption.isHidden());
    QVERIFY(loadOption.isForceReconnect());
    QCOMPARE(loadOption.category(), (quint8)(QEFI_LOAD_OPTION_CATEGORY_APP >> 8));

    // Changing category should not affect other flags
    loadOption.setCategory(0);
    QVERIFY(loadOption.isActive());
    QVERIFY(loadOption.isHidden());
    QVERIFY(loadOption.isForceReconnect());
    QCOMPARE(loadOption.category(), (quint8)0);

    // Changing other flags should not affect category
    loadOption.setCategory(QEFI_LOAD_OPTION_CATEGORY_APP >> 8);
    loadOption.setActive(false);
    loadOption.setHidden(false);
    QCOMPARE(loadOption.category(), (quint8)(QEFI_LOAD_OPTION_CATEGORY_APP >> 8));
}

void TestLoadOptionMutators::testAttributesFlags()
{
    QEFILoadOption loadOption;

    // Set multiple flags simultaneously
    loadOption.setActive(true);
    loadOption.setHidden(true);
    loadOption.setForceReconnect(true);

    QVERIFY(loadOption.isActive());
    QVERIFY(loadOption.isHidden());
    QVERIFY(loadOption.isForceReconnect());

    quint32 expectedAttrs = QEFI_LOAD_OPTION_ACTIVE
        | QEFI_LOAD_OPTION_HIDDEN
        | QEFI_LOAD_OPTION_FORCE_RECONNECT;
    QCOMPARE(loadOption.attributes() & expectedAttrs, expectedAttrs);

    // Use setAttributes directly
    loadOption.setAttributes(0x00000001);
    QCOMPARE(loadOption.attributes(), (quint32)0x00000001);
    QVERIFY(loadOption.isActive());
    QVERIFY(!loadOption.isHidden());
    QVERIFY(!loadOption.isForceReconnect());
}

void TestLoadOptionMutators::testDefaultConstructor()
{
    QEFILoadOption loadOption;

    QCOMPARE(loadOption.name(), QString());
    QVERIFY(!loadOption.isVisible());  // Default constructor: not visible until setActive
    QCOMPARE(loadOption.devicePathList().size(), 0);
    QVERIFY(loadOption.optionalData().isEmpty());
    QVERIFY(!loadOption.hasError());
    QVERIFY(loadOption.lastError().isEmpty());
    QVERIFY(!loadOption.isActive());
    QVERIFY(!loadOption.isHidden());
    QVERIFY(!loadOption.isForceReconnect());
    QCOMPARE(loadOption.category(), (quint8)0);
    QCOMPARE(loadOption.attributes(), (quint32)0);
}

void TestLoadOptionMutators::testCreateFromScratch()
{
    QEFILoadOption loadOption;

    loadOption.setName("Test Entry");
    loadOption.setIsVisible(true);

    // Add device paths using addDevicePath
    loadOption.addDevicePath(QSharedPointer<QEFIDevicePath>(
        new QEFIDevicePathACPIHID(0x0a0341d0, 0x00000001)
    ));

    // Verify the created entry
    QCOMPARE(loadOption.name(), QString("Test Entry"));
    QVERIFY(loadOption.isVisible());
    QCOMPARE(loadOption.devicePathList().size(), 1);

    // Verify format produces valid data
    QByteArray formatted = loadOption.format();
    QVERIFY(formatted.size() > 0);

    // Verify round-trip
    QEFILoadOption reparsed(formatted);
    QCOMPARE(reparsed.name(), QString("Test Entry"));
    QVERIFY(reparsed.isVisible());
}

void TestLoadOptionMutators::testHasError()
{
    // Valid data should have no error
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(!loadOption.hasError());

    // Empty data should have an error
    QByteArray empty;
    QEFILoadOption emptyOption(empty);
    QVERIFY(emptyOption.hasError());
    QVERIFY(!emptyOption.lastError().isEmpty());

    // clearError should reset state
    emptyOption.clearError();
    QVERIFY(!emptyOption.hasError());
}

void TestLoadOptionMutators::testLastError()
{
    // Valid data should have empty last error
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(loadOption.lastError().isEmpty());
}

void TestLoadOptionMutators::testIsValid()
{
    // Valid data should be valid
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(loadOption.isValid());

    // Empty data should not be valid
    QByteArray empty;
    QEFILoadOption emptyOption(empty);
    QVERIFY(!emptyOption.isValid());
}

void TestLoadOptionMutators::testTruncatedData()
{
    // Create truncated data (only first 10 bytes)
    QByteArray truncated((const char *)test_boot_data, 10);
    QEFILoadOption loadOption(truncated);

    // Should handle gracefully - must not crash, should have error
    QVERIFY(loadOption.hasError());
    QVERIFY(!loadOption.isValid());
}

void TestLoadOptionMutators::testCorruptedHeader()
{
    // Create data with corrupted header
    QByteArray corrupted = QByteArray((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    // Corrupt the device path length field (bytes 4-5)
    corrupted[4] = 0xFF;
    corrupted[5] = 0xFF;

    QEFILoadOption loadOption(corrupted);

    // Should handle gracefully - must not crash
    // Should either have error or be invalid
    QVERIFY(loadOption.hasError() || !loadOption.isValid());
}

QTEST_MAIN(TestLoadOptionMutators)

#include "test_load_option_mutators.moc"
