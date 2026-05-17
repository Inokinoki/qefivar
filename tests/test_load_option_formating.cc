#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestLoadOptionFormating: public QObject
{
    Q_OBJECT
private slots:
    void testReformatTestBootData();
    void testReformatTestBootData2();
};

void TestLoadOptionFormating::testReformatTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);
    QByteArray formatted = loadOption.format();
    QVERIFY(formatted.size() == TEST_BOOT_DATA_LENGTH);

    // Attribute: only support visibility
    QVERIFY((formatted[0] & QEFI_LOAD_OPTION_ACTIVE) == (char)0x01);

    // Verify device path length
    QVERIFY(formatted[4] == (char)test_boot_data[4]);
    QVERIFY(formatted[5] == (char)test_boot_data[5]);

    // Verify description
    int desc_len = strlen(test_boot_name) * 2;
    for (int i = 0; i < desc_len; i++) {
        QVERIFY(formatted[6 + i] == (char)test_boot_data[6 + i]);
    }
    QVERIFY(formatted[6 + desc_len] == (char)0x00);
    QVERIFY(formatted[6 + desc_len + 1] == (char)0x00);

    // Verify DP (Device Path) - starts after description + 2 null bytes
    int dp_offset = 6 + desc_len + 2;
    int dp_len = qFromLittleEndian<quint16>((quint8 *)&test_boot_data[4]);
    for (int i = 0; i < dp_len; i++) {
        QVERIFY(formatted[dp_offset + i] == (char)test_boot_data[dp_offset + i]);
    }

    // Verify optional data (should be empty for test_boot_data)
    QVERIFY(loadOption.optionalData().isEmpty());
}

void TestLoadOptionFormating::testReformatTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);
    QEFILoadOption loadOption(data);
    QByteArray formatted = loadOption.format();
    QVERIFY(formatted.size() == TEST_BOOT_DATA2_LENGTH);

    // Attribute: only support visibility
    QVERIFY((formatted[0] & QEFI_LOAD_OPTION_ACTIVE) == (char)0x01);

    // Verify device path length
    QVERIFY(formatted[4] == (char)test_boot_data2[4]);
    QVERIFY(formatted[5] == (char)test_boot_data2[5]);

    // Verify description
    int desc_len = strlen(test_boot_name2) * 2;
    for (int i = 0; i < desc_len; i++) {
        QVERIFY(formatted[6 + i] == (char)test_boot_data2[6 + i]);
    }
    QVERIFY(formatted[6 + desc_len] == (char)0x00);
    QVERIFY(formatted[6 + desc_len + 1] == (char)0x00);

    // Verify DP (Device Path) - starts after description + 2 null bytes
    int dp_offset = 6 + desc_len + 2;
    int dp_len = qFromLittleEndian<quint16>((quint8 *)&test_boot_data2[4]);
    for (int i = 0; i < dp_len; i++) {
        QVERIFY(formatted[dp_offset + i] == (char)test_boot_data2[dp_offset + i]);
    }

    // Verify optional data (Windows Boot Manager has optional data)
    QByteArray expectedOptionalData((const char *)&test_boot_data2[dp_offset + dp_len],
                                    TEST_BOOT_DATA2_LENGTH - (dp_offset + dp_len));
    QVERIFY(loadOption.optionalData() == expectedOptionalData);
}

QTEST_MAIN(TestLoadOptionFormating)

#include "test_load_option_formating.moc"