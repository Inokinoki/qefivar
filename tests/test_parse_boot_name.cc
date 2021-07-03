#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestParseBootName: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseEmptyData();
};

void TestParseBootName::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QString name = qefi_extract_name(data);
    QVERIFY(name == QString(test_boot_name));
}

void TestParseBootName::testParseEmptyData()
{
    QByteArray data;
    QString name = qefi_extract_name(data);
    QVERIFY(name.size() == 0);
}

QTEST_MAIN(TestParseBootName)

#include "test_parse_boot_name.moc"
