#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestLoadOptionHelpers: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseTestBootData2();
    void testParseEmptyData();
    void testFormatGuid();
    void testRfc4122ToGuid();
    void testLoadoptIsValid();
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED

void TestLoadOptionHelpers::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);

    QCOMPARE(qefi_loadopt_description_length(data),
        (int)(2 * strlen(test_boot_name)));
    QCOMPARE(qefi_loadopt_dp_list_length(data), 0x0068);
    QCOMPARE(qefi_loadopt_optional_data_length(data), 0);
}

void TestLoadOptionHelpers::testParseTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);

    QCOMPARE(qefi_loadopt_description_length(data),
        (int)(2 * strlen(test_boot_name2)));
    QCOMPARE(qefi_loadopt_dp_list_length(data), 0x0074);
    QCOMPARE(qefi_loadopt_optional_data_length(data), 136);
}

void TestLoadOptionHelpers::testParseEmptyData()
{
    QByteArray data;
    QVERIFY(qefi_loadopt_description_length(data) <= 0);
    QVERIFY(qefi_loadopt_dp_list_length(data) <= 0);
    QVERIFY(qefi_loadopt_optional_data_length(data) <= 0);
}

void TestLoadOptionHelpers::testFormatGuid()
{
    // Test GUID from test_boot_data GPT signature: d5 df 32 86 0f 91 3d 4b b2 50 2c 7f 17 44 15 45
    // This is a mixed-endian GUID format used by EFI
    quint8 rawGuid[16] = {
        0xd5, 0xdf, 0x32, 0x86, 0x0f, 0x91, 0x3d, 0x4b,
        0xb2, 0x50, 0x2c, 0x7f, 0x17, 0x44, 0x15, 0x45
    };
    QUuid result = qefi_format_guid(rawGuid);
    // qefi_format_guid reads as mixed-endian EFI GUID
    QVERIFY(!result.isNull());
}

void TestLoadOptionHelpers::testRfc4122ToGuid()
{
    // Test with a known GUID
    QUuid testUuid = QUuid("{8632dfd5-910f-4b3d-b250-2c7f17441545}");
    QByteArray rfcData = testUuid.toRfc4122();
    QCOMPARE(rfcData.size(), 16);

    QByteArray result = qefi_rfc4122_to_guid(rfcData);
    QCOMPARE(result.size(), 16);
}

void TestLoadOptionHelpers::testLoadoptIsValid()
{
    // Valid data should be valid
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QVERIFY(qefi_loadopt_is_valid(data));

    // Empty data should not be valid
    QVERIFY(!qefi_loadopt_is_valid(QByteArray()));

    // Truncated data should not be valid
    QByteArray truncated((const char *)test_boot_data, 4);
    QVERIFY(!qefi_loadopt_is_valid(truncated));
}

QT_WARNING_POP

QTEST_MAIN(TestLoadOptionHelpers)

#include "test_load_option_helpers.moc"
