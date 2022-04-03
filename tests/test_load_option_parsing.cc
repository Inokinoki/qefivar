#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestLoadOptionParsing: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseEmptyData();
};

void TestLoadOptionParsing::testParseTestBootData()
{
    QByteArray data((const char *)test_boot_data, TEST_BOOT_DATA_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(loadOption.isVisible() == true);
    QVERIFY(loadOption.name() == QString(test_boot_name));
    QVERIFY(loadOption.path() == QString(test_boot_path));
    QVERIFY(loadOption.devicePathList().size() == 2);
}

void TestLoadOptionParsing::testParseEmptyData()
{
    QByteArray data;
    QEFILoadOption loadOption(data);
    QVERIFY(loadOption.name().size() == 0);
    QVERIFY(loadOption.path().size() == 0);
    QVERIFY(loadOption.devicePathList().size() == 0);
}

QTEST_MAIN(TestLoadOptionParsing)

#include "test_load_option_parsing.moc"