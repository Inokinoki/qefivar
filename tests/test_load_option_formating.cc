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
    QVERIFY(formatted[0] & QEFI_LOAD_OPTION_ACTIVE == (char)0x01);

    // Verify device path length
    QVERIFY(formatted[4] == (char)test_boot_data[4]);
    QVERIFY(formatted[5] == (char)test_boot_data[5]);

    // Verify descciption
    int desc_len = strlen(test_boot_name) * 2;
    for (int i = 0; i < desc_len; i++) {
        QVERIFY(formatted[6 + i] == (char)test_boot_data[6 + i]);
    }
    QVERIFY(formatted[6 + desc_len] == (char)0x00);
    QVERIFY(formatted[6 + desc_len + 1] == (char)0x00);

    // TODO: Verify DP
    // TODO: Verify optional data
}

void TestLoadOptionFormating::testReformatTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);
    QEFILoadOption loadOption(data);
    QByteArray formatted = loadOption.format();
    QVERIFY(formatted.size() == TEST_BOOT_DATA2_LENGTH);

    // Attribute: only support visibility
    QVERIFY(formatted[0] & QEFI_LOAD_OPTION_ACTIVE == (char)0x01);

    // Verify device path length
    QVERIFY(formatted[4] == (char)test_boot_data2[4]);
    QVERIFY(formatted[5] == (char)test_boot_data2[5]);

    // Verify descciption
    int desc_len = strlen(test_boot_name2) * 2;
    for (int i = 0; i < desc_len; i++) {
        QVERIFY(formatted[6 + i] == (char)test_boot_data2[6 + i]);
    }
    QVERIFY(formatted[6 + desc_len] == (char)0x00);
    QVERIFY(formatted[6 + desc_len + 1] == (char)0x00);

    // TODO: Verify DP
    // TODO: Verify optional data
}

QTEST_MAIN(TestLoadOptionFormating)

#include "test_load_option_formating.moc"