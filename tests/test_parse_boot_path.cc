#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestParseBootPath: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseEmptyData();
};

void TestParseBootPath::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QString path = qefi_extract_path(data);
    QVERIFY(path == QString(test_boot_path));
}

void TestParseBootPath::testParseEmptyData()
{
    QByteArray data;
    QString path = qefi_extract_path(data);
    QVERIFY(path.size() == 0);
}

QTEST_MAIN(TestParseBootPath)

#include "test_parse_boot_path.moc"
