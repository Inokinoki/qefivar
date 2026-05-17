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

QT_WARNING_POP

QTEST_MAIN(TestLoadOptionHelpers)

#include "test_load_option_helpers.moc"
