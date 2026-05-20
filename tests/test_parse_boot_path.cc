#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestParseBootPath: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseTestBootData2();
    void testParseEmptyData();
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED

void TestParseBootPath::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QString path = qefi_extract_path(data);
    QCOMPARE(path, QString(test_boot_path));
}

void TestParseBootPath::testParseTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);
    QString path = qefi_extract_path(data);
    QCOMPARE(path, QString(test_boot_path2));
}

void TestParseBootPath::testParseEmptyData()
{
    QByteArray data;
    QString path = qefi_extract_path(data);
    QCOMPARE(path.size(), 0);
}

QT_WARNING_POP

QTEST_MAIN(TestParseBootPath)

#include "test_parse_boot_path.moc"
