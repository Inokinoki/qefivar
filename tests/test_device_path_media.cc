
#include <QtTest/QtTest>
#include <QSharedPointer>

#include "test_data.h"
#include "../qefi.h"

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
};

/* EFI device path header */
#pragma pack(push, 1)
struct qefi_device_path_header {
    quint8 type;
    quint8 subtype;
    quint16 length;
};
#pragma pack(pop)

// Hardware parsing
QEFIDevicePath *qefi_parse_dp_media_file(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_hdd(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_cdrom(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_vendor(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_protocol(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_firmware_file(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_fv(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_relative_offset(
    struct qefi_device_path_header *dp, int dp_size);
QEFIDevicePath *qefi_parse_dp_media_ramdisk(
    struct qefi_device_path_header *dp, int dp_size);

// Format Media
QByteArray qefi_format_dp_media_hdd(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_file(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_cdrom(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_vendor(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_protocol(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_firmware_file(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_fv(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_relative_offset(QEFIDevicePath *dp);
QByteArray qefi_format_dp_media_ramdisk(QEFIDevicePath *dp);

void TestDevicePathMedia::test_qefi_dp_media_file()
{
    QEFIDevicePathMediaFile dp(
        QStringLiteral("\\EFI\\refind\\refind_x64.efi"));
    QByteArray data = qefi_format_dp_media_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_File);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_file(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaFile *subP =
        dynamic_cast<QEFIDevicePathMediaFile *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->name() == dp.name());
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
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_HD);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_hdd(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaHD *subP =
        dynamic_cast<QEFIDevicePathMediaHD *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->partitionNumber() == dp.partitionNumber());
    QVERIFY(subP->start() == dp.start());
    QVERIFY(subP->size() == dp.size());
    QVERIFY(subP->gptGuid() == dp.gptGuid());
    QVERIFY(subP->format() == dp.format());
    QVERIFY(subP->signatureType() == dp.signatureType());
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
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_CDROM);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_cdrom(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaCDROM *subP =
        dynamic_cast<QEFIDevicePathMediaCDROM *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->bootCatalogEntry() == dp.bootCatalogEntry());
    QVERIFY(subP->partitionRba() == dp.partitionRba());
    QVERIFY(subP->sectors() == dp.sectors());
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
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_Vendor);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_vendor(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaVendor *subP =
        dynamic_cast<QEFIDevicePathMediaVendor *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->vendorGuid() == dp.vendorGuid());
    QVERIFY(subP->vendorData() == dp.vendorData());
}

void TestDevicePathMedia::test_qefi_dp_media_protocol()
{
    QEFIDevicePathMediaProtocol dp(
        /* uuid */QUuid("df98065f-0102-4255-8d88-dfd07e3e1629"));
    QByteArray data = qefi_format_dp_media_protocol((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_Protocol);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_protocol(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaProtocol *subP =
        dynamic_cast<QEFIDevicePathMediaProtocol *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->protocolGuid() == dp.protocolGuid());
}

void TestDevicePathMedia::test_qefi_dp_media_firmware_file()
{
    QEFIDevicePathMediaFirmwareFile dp(/* piData */QByteArray(514, 114));
    QByteArray data =
        qefi_format_dp_media_firmware_file((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_FirmwareFile);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_firmware_file(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaFirmwareFile *subP =
        dynamic_cast<QEFIDevicePathMediaFirmwareFile *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->piInfo() == dp.piInfo());
}

void TestDevicePathMedia::test_qefi_dp_media_fv()
{
    QEFIDevicePathMediaFirmwareVolume dp(/* piData */QByteArray(514, 114));
    QByteArray data =
        qefi_format_dp_media_fv((QEFIDevicePath *)&dp);

    struct qefi_device_path_header *dp_header =
        (struct qefi_device_path_header *)data.data();
    // Test format
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_FirmwareVolume);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_fv(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaFirmwareVolume *subP =
        dynamic_cast<QEFIDevicePathMediaFirmwareVolume *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->piInfo() == dp.piInfo());
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
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_RelativeOffset);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_relative_offset(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaRelativeOffset *subP =
        dynamic_cast<QEFIDevicePathMediaRelativeOffset *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->firstByte() == dp.firstByte());
    QVERIFY(subP->lastByte() == dp.lastByte());
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
    QVERIFY(dp_header->type == QEFIDevicePathType::DP_Media);
    QVERIFY(dp_header->subtype == QEFIDevicePathMediaSubType::MEDIA_RamDisk);
    QVERIFY(qFromLittleEndian<quint16>(dp_header->length) == data.length());

    // Test parse
    QSharedPointer<QEFIDevicePath> p(
        qefi_parse_dp_media_ramdisk(dp_header, data.length()));
    QVERIFY(p->type() == dp.type());
    QVERIFY(p->subType() == dp.subType());
    QEFIDevicePathMediaRAMDisk *subP =
        dynamic_cast<QEFIDevicePathMediaRAMDisk *>(p.get());
    QVERIFY(subP != nullptr);
    QVERIFY(subP->startAddress() == dp.startAddress());
    QVERIFY(subP->endAddress() == dp.endAddress());
    QVERIFY(subP->diskTypeGuid() == dp.diskTypeGuid());
    QVERIFY(subP->instanceNumber() == dp.instanceNumber());
}

QTEST_MAIN(TestDevicePathMedia)

#include "test_device_path_media.moc"
