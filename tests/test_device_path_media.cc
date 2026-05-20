
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi_p.h"

class TestDevicePathMedia: public QObject
{
    Q_OBJECT
private slots:
    void test_qefi_dp_media_file();
    void test_qefi_dp_media_hdd();
    void test_qefi_dp_media_cdrom();
    void test_qefi_dp_media_vendor();
    void test_qefi_dp_media_protocol();
    void test_qefi_dp_media_firmware_file();
    void test_qefi_dp_media_fv();
    void test_qefi_dp_media_relative_offset();
    void test_qefi_dp_media_ramdisk();
    void test_qefi_dp_media_hdd_mbr_format();
    void test_qefi_dp_media_hdd_gpt_format();
    void test_qefi_dp_media_file_empty_path();
    void test_qefi_dp_media_file_long_path();
    void test_qefi_dp_media_ramdisk_boundary();
    void test_qefi_parse_dp_generic_media_file();
};

void TestDevicePathMedia::test_qefi_dp_media_file()
{
    QEFIDevicePathMediaFile dp(
        QStringLiteral("\\EFI\\refind\\refind_x64.efi"));
    QByteArray data = qefi_format_dp_media_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_File);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_file(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaFile *subP =
        dynamic_cast<QEFIDevicePathMediaFile *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->name(), dp.name());
}

void TestDevicePathMedia::test_qefi_dp_media_hdd()
{
    QByteArray guid = qefi_rfc4122_to_guid(
        QUuid("df98065f-0102-4255-8d88-dfd07e3e1629").toRfc4122());
    QEFIDevicePathMediaHD dp(/* partitionNumber */ 1, /* start */ 1024,
        /* size */ 1024 * 1024 * 100, /* signature */ (quint8 *)guid.data(),
        /* format */
        QEFIDevicePathMediaHD::QEFIDevicePathMediaHDFormat::GPT,
        /* signatureType */
        QEFIDevicePathMediaHD::QEFIDevicePathMediaHDSignatureType::GUID);
    QByteArray data = qefi_format_dp_media_hdd((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_HD);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_hdd(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaHD *subP =
        dynamic_cast<QEFIDevicePathMediaHD *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->partitionNumber(), dp.partitionNumber());
    QCOMPARE(subP->start(), dp.start());
    QCOMPARE(subP->size(), dp.size());
    QCOMPARE(subP->gptGuid(), dp.gptGuid());
    QCOMPARE(subP->format(), dp.format());
    QCOMPARE(subP->signatureType(), dp.signatureType());
}

void TestDevicePathMedia::test_qefi_dp_media_cdrom()
{
    QEFIDevicePathMediaCDROM dp(/* entry */ 0xA5A5A5A5,
        /* partitionRba */ 0x5A5A5A5A5A5A5A5A,
        /* sectors */ 0x2222233333114514);
    QByteArray data = qefi_format_dp_media_cdrom((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_CDROM);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_cdrom(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaCDROM *subP =
        dynamic_cast<QEFIDevicePathMediaCDROM *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->bootCatalogEntry(), dp.bootCatalogEntry());
    QCOMPARE(subP->partitionRba(), dp.partitionRba());
    QCOMPARE(subP->sectors(), dp.sectors());
}

void TestDevicePathMedia::test_qefi_dp_media_vendor()
{
    QEFIDevicePathMediaVendor dp(
        /* uuid */QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"),
        /* data */QByteArray(514, 114));
    QByteArray data = qefi_format_dp_media_vendor((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_Vendor);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_vendor(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaVendor *subP =
        dynamic_cast<QEFIDevicePathMediaVendor *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->vendorGuid(), dp.vendorGuid());
    QCOMPARE(subP->vendorData(), dp.vendorData());
}

void TestDevicePathMedia::test_qefi_dp_media_protocol()
{
    QEFIDevicePathMediaProtocol dp(
        /* uuid */QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"));
    QByteArray data = qefi_format_dp_media_protocol((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_Protocol);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_protocol(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaProtocol *subP =
        dynamic_cast<QEFIDevicePathMediaProtocol *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->protocolGuid(), dp.protocolGuid());
}

void TestDevicePathMedia::test_qefi_dp_media_firmware_file()
{
    QEFIDevicePathMediaFirmwareFile dp(/* piData */QByteArray(514, 114));
    QByteArray data =
        qefi_format_dp_media_firmware_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_FirmwareFile);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_firmware_file(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaFirmwareFile *subP =
        dynamic_cast<QEFIDevicePathMediaFirmwareFile *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->piInfo(), dp.piInfo());
}

void TestDevicePathMedia::test_qefi_dp_media_fv()
{
    QEFIDevicePathMediaFirmwareVolume dp(/* piData */QByteArray(514, 114));
    QByteArray data =
        qefi_format_dp_media_fv((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_fv(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaFirmwareVolume *subP =
        dynamic_cast<QEFIDevicePathMediaFirmwareVolume *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->piInfo(), dp.piInfo());
}

void TestDevicePathMedia::test_qefi_dp_media_relative_offset()
{
    QEFIDevicePathMediaRelativeOffset dp(/* reserved */ 0x0,
        /* firstByte */ 0x5A5A5A5A5A5A5A5A,
        /* lastByte */ 0x2222233333114514);
    QByteArray data = qefi_format_dp_media_relative_offset((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_RelativeOffset);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_relative_offset(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaRelativeOffset *subP =
        dynamic_cast<QEFIDevicePathMediaRelativeOffset *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->firstByte(), dp.firstByte());
    QCOMPARE(subP->lastByte(), dp.lastByte());
}

void TestDevicePathMedia::test_qefi_dp_media_ramdisk()
{
    QEFIDevicePathMediaRAMDisk dp(
        /* startAddress */ 0x5A5A5A5A5A5A5A5A,
        /* endAddress */ 0x2222233333114514,
        /* diskTypeGuid */ QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"),
        /* instanceNumber */ 0x8877);
    QByteArray data = qefi_format_dp_media_ramdisk((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QCOMPARE(dp_header->type, QEFIDevicePathType::DP_Media);
    QCOMPARE(dp_header->subtype, QEFIDevicePathMediaSubType::MEDIA_RamDisk);
    QCOMPARE(qFromLittleEndian<quint16>(dp_header->length), (quint16)data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_ramdisk(dp_header, data.length()));
    QCOMPARE(p->type(), dp.type());
    QCOMPARE(p->subType(), dp.subType());
    QEFIDevicePathMediaRAMDisk *subP =
        dynamic_cast<QEFIDevicePathMediaRAMDisk *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->startAddress(), dp.startAddress());
    QCOMPARE(subP->endAddress(), dp.endAddress());
    QCOMPARE(subP->diskTypeGuid(), dp.diskTypeGuid());
    QCOMPARE(subP->instanceNumber(), dp.instanceNumber());
}

void TestDevicePathMedia::test_qefi_dp_media_hdd_mbr_format()
{
    quint8 mbrSignature[16] = {0};
    mbrSignature[0] = 0x01;
    mbrSignature[1] = 0x02;
    mbrSignature[2] = 0x03;
    mbrSignature[3] = 0x04;

    QEFIDevicePathMediaHD dp(
        /* partitionNumber */ 1,
        /* start */ 2048,
        /* size */ 1024 * 1024 * 100,
        /* signature */ mbrSignature,
        /* format */ QEFIDevicePathMediaHD::PCAT,
        /* signatureType */ QEFIDevicePathMediaHD::MBR);

    QByteArray data = qefi_format_dp_media_hdd((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_hdd(dp_header, data.length()));
    QEFIDevicePathMediaHD *subP =
        dynamic_cast<QEFIDevicePathMediaHD *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->format(), QEFIDevicePathMediaHD::PCAT);
    QCOMPARE(subP->signatureType(), QEFIDevicePathMediaHD::MBR);
    QCOMPARE(subP->mbrSignature(), (quint32)0x04030201);
}

void TestDevicePathMedia::test_qefi_dp_media_hdd_gpt_format()
{
    QByteArray guid = qefi_rfc4122_to_guid(
        QUuid("12345678-1234-1234-1234-123456789ABC").toRfc4122());

    QEFIDevicePathMediaHD dp(
        /* partitionNumber */ 2,
        /* start */ 4096,
        /* size */ 1024 * 1024 * 200,
        /* signature */ (quint8 *)guid.data(),
        /* format */ QEFIDevicePathMediaHD::GPT,
        /* signatureType */ QEFIDevicePathMediaHD::GUID);

    QByteArray data = qefi_format_dp_media_hdd((QEFIDevicePath *)&dp);
    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_hdd(dp_header, data.length()));
    QEFIDevicePathMediaHD *subP =
        dynamic_cast<QEFIDevicePathMediaHD *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->format(), QEFIDevicePathMediaHD::GPT);
    QCOMPARE(subP->signatureType(), QEFIDevicePathMediaHD::GUID);
    QCOMPARE(subP->partitionNumber(), (quint32)2);
}

void TestDevicePathMedia::test_qefi_dp_media_file_empty_path()
{
    QEFIDevicePathMediaFile dp("");
    QByteArray data = qefi_format_dp_media_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_file(dp_header, data.length()));
    QEFIDevicePathMediaFile *subP =
        dynamic_cast<QEFIDevicePathMediaFile *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->name().isEmpty());
}

void TestDevicePathMedia::test_qefi_dp_media_file_long_path()
{
    QString longPath = "\\EFI\\Boot\\bootx64.efi";
    QEFIDevicePathMediaFile dp(longPath);
    QByteArray data = qefi_format_dp_media_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();

    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_file(dp_header, data.length()));
    QEFIDevicePathMediaFile *subP =
        dynamic_cast<QEFIDevicePathMediaFile *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->name(), longPath);
}

void TestDevicePathMedia::test_qefi_dp_media_ramdisk_boundary()
{
    {
        QEFIDevicePathMediaRAMDisk dp(
            /* startAddress */ 0x10000000,
            /* endAddress */ 0x1FFFFFFF,
            /* diskTypeGuid */ QUuid("12345678-1234-1234-1234-123456789ABC"),
            /* instanceNumber */ 0x0000);

        QByteArray data = qefi_format_dp_media_ramdisk((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_media_ramdisk(dp_header, data.length()));
        QEFIDevicePathMediaRAMDisk *subP =
            dynamic_cast<QEFIDevicePathMediaRAMDisk *>(p.get());
        QVERIFY(subP != nullptr);
        QCOMPARE(subP->startAddress(), (quint64)0x10000000);
        QCOMPARE(subP->endAddress(), (quint64)0x1FFFFFFF);
        QCOMPARE(subP->instanceNumber(), (quint16)0x0000);
    }

    {
        QEFIDevicePathMediaRAMDisk dp(
            /* startAddress */ 0x0,
            /* endAddress */ 0xFFFFFFFFFFFFFFFF,
            /* diskTypeGuid */ QUuid("87654321-4321-4321-4321-CBA987654321"),
            /* instanceNumber */ 0xFFFF);

        QByteArray data = qefi_format_dp_media_ramdisk((QEFIDevicePath *)&dp);
        struct qefi_device_path_header *dp_header =
            (struct qefi_device_path_header *)data.data();

        QSharedPointer<QEFIDevicePath> p(
            qefi_parse_dp_media_ramdisk(dp_header, data.length()));
        QEFIDevicePathMediaRAMDisk *subP =
            dynamic_cast<QEFIDevicePathMediaRAMDisk *>(p.get());
        QVERIFY(subP != nullptr);
        QCOMPARE(subP->startAddress(), (quint64)0x0);
        QCOMPARE(subP->endAddress(), (quint64)0xFFFFFFFFFFFFFFFF);
        QCOMPARE(subP->instanceNumber(), (quint16)0xFFFF);
    }
}

void TestDevicePathMedia::test_qefi_parse_dp_generic_media_file()
{
    QEFIDevicePathMediaFile dp(QStringLiteral("\\EFI\\Boot\\bootx64.efi"));
    QByteArray data = qefi_format_dp((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp(dp_header, data.length()));
    QVERIFY(p != nullptr);
    QCOMPARE(p->type(), QEFIDevicePathType::DP_Media);
    QCOMPARE(p->subType(), QEFIDevicePathMediaSubType::MEDIA_File);
    QEFIDevicePathMediaFile *subP =
        dynamic_cast<QEFIDevicePathMediaFile *>(p.get());
    QVERIFY(subP != nullptr);
    QCOMPARE(subP->name(), QString("\\EFI\\Boot\\bootx64.efi"));
}

QTEST_MAIN(TestDevicePathMedia)

#include "test_device_path_media.moc"
