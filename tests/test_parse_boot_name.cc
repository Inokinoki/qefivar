#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestParseBootName: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseTestBootData2();
    void testParseEmptyData();
};

void TestParseBootName::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QString name = qefi_extract_name(data);
    QVERIFY(name == QString(test_boot_name));
}

void TestParseBootName::testParseTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);
    QString name = qefi_extract_name(data);
    QVERIFY(name == QString(test_boot_name2));
}

void TestParseBootName::testParseEmptyData()
{
    QByteArray data;
    QString name = qefi_extract_name(data);
    QVERIFY(name.size() == 0);
}

QTEST_MAIN(TestParseBootName)

#include "test_parse_boot_name.moc"
