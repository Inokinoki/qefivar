
#include <QtTest/QtTest>
#include <QDebug>
#include <QDir>

#include "test_data.h"
#include "../qefi.h"

class TestDummyBackend : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test_qefi_data_read_write();
    void test_qefi_uint16_read_write();
    void cleanupTestCase();
};

bool dummy_backend_get_dir(QString &dir);

void TestDummyBackend::initTestCase()
{
    QVERIFY(qefi_is_available());
    QVERIFY(qefi_has_privilege());

    QString dir;
    QVERIFY(dummy_backend_get_dir(dir));
    QVERIFY(QDir(dir).exists());
}

void TestDummyBackend::cleanupTestCase()
{
    QString dir;
    if (dummy_backend_get_dir(dir)) {
        QDir(dir).removeRecursively();
    }
}

void TestDummyBackend::test_qefi_data_read_write()
{
    QByteArray data;
    data.append((char)0x12);
    data.append((char)0x00);
    data.append((char)0x13);
    data.append((char)0x00);
    data.append((char)0x01);
    data.append((char)0x20);
    qefi_set_variable(QUuid(), QStringLiteral("BootOrder"), data);
    QByteArray res = qefi_get_variable(QUuid(), QStringLiteral("BootOrder"));
    QVERIFY(res == data);
}

void TestDummyBackend::test_qefi_uint16_read_write()
{
    qefi_set_variable_uint16(QUuid(), QStringLiteral("BootNext"), 0xFFEE);
    quint16 res = qefi_get_variable_uint16(QUuid(), QStringLiteral("BootNext"));
    QVERIFY(res == 0xFFEE);
}

QTEST_MAIN(TestDummyBackend)

#include "test_dummy_backend.moc"
