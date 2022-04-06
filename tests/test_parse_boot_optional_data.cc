#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestParseBootOptionalData: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseTestBootData2();
    void testParseEmptyData();
};

void TestParseBootOptionalData::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QByteArray optionalData = qefi_extract_optional_data(data);
    QVERIFY(optionalData.size() == 0);
}

void TestParseBootOptionalData::testParseTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);
    QByteArray optionalData = qefi_extract_optional_data(data);
    QVERIFY(optionalData.size() == 136);
}

void TestParseBootOptionalData::testParseEmptyData()
{
    QByteArray data;
    QByteArray optionalData = qefi_extract_optional_data(data);
    QVERIFY(optionalData.size() == 0);
}

QTEST_MAIN(TestParseBootOptionalData)

#include "test_parse_boot_optional_data.moc"
