#include <QtTest/QtTest>

#include "test_data.h"
#include "../qefi.h"

class TestLoadOptionParsing: public QObject
{
    Q_OBJECT
private slots:
    void testParseTestBootData();
    void testParseTestBootData2();
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

    const auto &devicePathList = loadOption.devicePathList();
    for (const auto &dp : devicePathList) {
        if (dp->type() == QEFIDevicePathType::DP_Media &&
            dp->subType() == QEFIDevicePathMediaSubType::MEDIA_HD) {
            QEFIDevicePathMediaHD *dpMediaHD =
                dynamic_cast<QEFIDevicePathMediaHD *>(dp.get());
            QVERIFY(dpMediaHD != nullptr);
            QVERIFY(dpMediaHD->signatureType() ==
                QEFIDevicePathMediaHD::QEFIDevicePathMediaHDSignatureType::GUID);
            QVERIFY(!dpMediaHD->gptGuid().isNull());
            QVERIFY(dpMediaHD->gptGuid() ==
                QUuid("8632dfd5-910f-4b3d-b250-2c7f17441545"));
            break;
        }
    }
}

void TestLoadOptionParsing::testParseTestBootData2()
{
    QByteArray data((const char *)test_boot_data2, TEST_BOOT_DATA2_LENGTH);
    QEFILoadOption loadOption(data);
    QVERIFY(loadOption.isVisible() == true);
    QVERIFY(loadOption.name() == QString(test_boot_name2));
    QVERIFY(loadOption.path() == QString(test_boot_path2));
    QVERIFY(loadOption.devicePathList().size() == 2);
    const auto &devicePathList = loadOption.devicePathList();
    for (const auto &dp : devicePathList) {
        if (dp->type() == QEFIDevicePathType::DP_Media &&
            dp->subType() == QEFIDevicePathMediaSubType::MEDIA_HD) {
            QEFIDevicePathMediaHD *dpMediaHD =
                dynamic_cast<QEFIDevicePathMediaHD *>(dp.get());
            QVERIFY(dpMediaHD != nullptr);
            QVERIFY(dpMediaHD->signatureType() ==
                QEFIDevicePathMediaHD::QEFIDevicePathMediaHDSignatureType::GUID);
            QVERIFY(!dpMediaHD->gptGuid().isNull());
            QVERIFY(dpMediaHD->gptGuid() ==
                QUuid("8632dfd5-910f-4b3d-b250-2c7f17441545"));
            break;
        }
    }
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
