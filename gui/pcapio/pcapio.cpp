#include "pcapio.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <climits>

PcapPacket::PcapPacket(QByteArray data) :
    data(data)
{
    microsec = 0;
    setTimestamp(QDateTime::currentDateTime());
    originalSize = data.size();
    capturedSize = data.size();
}
quint32 PcapPacket::getTimestampRaw() const
{
    return timestamp;
}

QDateTime PcapPacket::getTimestamp() const {

    QDateTime val = QDateTime::fromTime_t(timestamp);

    // loosing precision here as QDateTime does not go down to microseconds.
    val = val.addMSecs((qint64) microsec / 1000);
    return val;
}

void PcapPacket::setTimestamp(const quint32 &value)
{
    timestamp = value;
}

void PcapPacket::setTimestamp(const QDateTime &value)
{
    timestamp = value.toTime_t();
    // QTime garantee that msec() returns a value between 0 and 999, so no risk of overflow here
    int msec = value.time().msec();
    // checking anyway ;P
    if (msec < 0 || msec > 999) {
        qCritical() << QObject::tr("QDateTime::time().msec() returned an incorrect value: %1 T_T").arg(msec);
        msec = 0;
    }
    microsec  = (quint32) msec * 1000;
}
quint32 PcapPacket::getMicrosec() const
{
    return microsec;
}

void PcapPacket::setMicrosec(const quint32 &value)
{
    microsec = value;
}
quint32 PcapPacket::getOriginalSize() const
{
    return originalSize;
}

void PcapPacket::setOriginalSize(const quint32 &value)
{
    originalSize = value;
}
quint32 PcapPacket::getCapturedSize() const
{
    return capturedSize;
}

void PcapPacket::setCapturedSize(const quint32 &value)
{
    capturedSize = value;
}
QByteArray PcapPacket::getData() const
{
    return data;
}

void PcapPacket::setData(const QByteArray &value)
{
    data = value;
    originalSize = data.size();
}

const QByteArray PcapIO::PCAP_MAGIC_LE = "\xd4\xc3\xb2\xa1";
const QByteArray PcapIO::PCAP_MAGIC_BE = "\xa1\xb2\xc3\xd4";

PcapIO::PcapIO(QString filename, QObject *parent) :
    QObject(parent),
    filename(filename)
{
    ownFileObject = true;
    initAttributes();
}

PcapIO::PcapIO(QIODevice *device, QObject *parent) :
    QObject(parent),
    file (device)
{
    ownFileObject = false;
    initAttributes();
}

PcapIO::~PcapIO()
{
    if (ownFileObject)
        delete file;
    file = nullptr;
}

bool PcapIO::openExistingFile(QIODevice::OpenModeFlag mode)
{
    if (!filename.isEmpty()) {
        file = new(std::nothrow) QFile(filename);
        if (file == nullptr) {
            qFatal("[PcapIO::open] Cannot allocate memory for QFile");
        }

        if (!file->open(mode)) {
            qCritical() << "[PcapIO::openExistingFile] Cannot open file:" << file->errorString();
            return false;
        }

    } else {

    }

    if (file->isReadable()) {
        if (!readExistingFile()) {

            if (ownFileObject) {
                file->close();
                delete file;
            }
            initAttributes();
            return false;
        }
    } else {
        qCritical() << "[PcapIO::openExistingFile] Cannot read file";
        return false;
    }

    return true;

}

bool PcapIO::createAndOpenFile()
{
    if (!filename.isEmpty()) {
        file = new(std::nothrow) QFile(filename);
        if (file == nullptr) {
            qFatal("[PcapIO::createAndOpenNewFile] Cannot allocate memory for QFile");
        }

        if (!file->open(QIODevice::ReadWrite)) {
            qCritical() << "[PcapIO::createAndOpenNewFile] Cannot open file:" << file->errorString();
            return false;
        }
    }

    if (!file->isWritable()) {
        qCritical() << "[PcapIO::createAndOpenNewFile] Cannot write to file";
        return false;
    }

    QByteArray data = generatePcapHeader();
    if (data.size() > 0) {
        file->write(data);
        currentPos = file->pos();
        packetsStart = currentPos;
    } else {
        qCritical() << "[PcapIO::createAndOpenNewFile] whhaaa ? the generated pcap header is empty T_T";
        return false;
    }

    return true;
}

void PcapIO::close()
{
    if (file != nullptr) {
        if (file->isOpen())
            file->close();
        if (ownFileObject)
            delete file;
        initAttributes();
    }
}

QByteArray PcapIO::reverseBytesOrder(QByteArray &in)
{
    QByteArray ret;
    ret.resize(in.size());
    for(int i=in.size(); i>=0; --i)
        ret.append(in.at(i));
    return ret;
}

quint64 PcapIO::numberOfPackets()
{
    if (numberOfPacketsInFile > 0)
        return numberOfPacketsInFile;

    quint64 ret = 0;
    if (file != nullptr && file->isOpen()) {
        if (file->seek(packetsStart)) {
            PcapPacket * packet = nextPacket();
            while (packet != nullptr) {
                delete packet;
                packet = nextPacket();
                ret++;
            }
        } else {
            qCritical() << "[PcapIO::numberOfPackets] Cannot seek the file to the start of the packets";
        }
    }

    numberOfPacketsInFile = ret;
    return ret;
}

PcapPacket *PcapIO::nextPacket()
{
    PcapPacket * ret = new(std::nothrow) PcapPacket();
    if (ret == nullptr) {
        qFatal("[PcapIO::nextPacket] Cannot allocate memory for PcapPacket");
    }



    if (file->seek(currentPos)) {
        if (file->atEnd()) {
            delete ret;
            return nullptr;
        }
        quint32 temp = 0;
        quint32 dataSize = 0;

        // start with the timestamp

        if (!readUInt32(&temp, QString("Packet timestamp"))) {
            goto error;
        }

        ret->setTimestamp(temp);

        // microseconds

        if (!readUInt32(&temp, QString("Packet microseconds"))) {
            goto error;
        }

        ret->setMicrosec(temp);

        // captured packet size
        if (!readUInt32(&temp, QString("Packet capture size"))) {
            goto error;
        }

        ret->setCapturedSize(temp);
        dataSize = temp;

        // original packet size
        if (!readUInt32(&temp, QString("Packet original size"))) {
            goto error;
        }

        ret->setOriginalSize(temp);


        // get the data blob
        QByteArray data;
        if (!readByteArray(&data,dataSize, QString("Packet data"))) {
            goto error;
        }

        ret->setData(data);

        currentPos = file->pos();

    } else {
        qCritical() << "[PcapIO::nextPacket] Cannot seek the file to the current packet";
        goto error;
    }

    return ret;

    error:
        delete ret;
        return nullptr;
}

void PcapIO::appendPacket(PcapPacket *packet)
{
    if (file != nullptr && file->isWritable()) {
        if (!file->atEnd()) // we are appending ...
            file->seek(file->size());

        file->write(packetToBytes(packet));
    } else {
        qCritical() << "[PcapIO::appendPacket] File is nullptr or not writeable";
    }
}

void PcapIO::appendPacketList(QList<PcapPacket *> list)
{
    if (file != nullptr && file->isWritable()) {
        if (!file->atEnd()) // we are appending ...
            file->seek(file->size());

        for (int i = 0; i < list.size(); i++) {
            file->write(packetToBytes(list.at(i)));
        }

    } else {
        qCritical() << "[PcapIO::appendPacketList] File is nullptr or not writeable";
    }

}

QList<PcapPacket *> PcapIO::getPacketList()
{
    QList<PcapPacket *> list;
    qint64 currentPostTemp = currentPos;

    if (file != nullptr && file->isOpen()) {
        if (file->seek(packetsStart)) {
            currentPos = packetsStart;
            PcapPacket * packet = nextPacket();
            while (packet != nullptr) {
                list.append(packet);
                packet = nextPacket();
            }
        } else {
            qCritical() << "[PcapIO::getPacketList] Cannot seek the file to the start of the packets";
        }
    } else {
        qCritical() << "[PcapIO::getPacketList] File is nullptr or not readable";
    }

    currentPos = currentPostTemp;

    return list;
}

void PcapIO::initAttributes()
{
    if (ownFileObject)
        file = nullptr;
    versionMajor = 2;
    versionMinor = 4;
    reverseOrder = false;
#ifdef Q_LITTLE_ENDIAN
    littleEndian = true;
#else
    littleEndian = false;
#endif
    timezoneshift = 0;
    precision = 0;
    maxSnapshotlenght = INT_MAX; // we could use UINT_MAX here but it is unclear if this number is unsigned or signed
    linkLayerType = PcapDef::LINKTYPE_nullptr;
    packetsStart = 0;
    currentPos = 0;
    numberOfPacketsInFile = 0;
}

bool PcapIO::readUInt32(quint32 *temp, QString field)
{
    QByteArray fourBytes(sizeof(quint32), '\0');
    qint64 read = file->read(fourBytes.data(), sizeof(quint32));
    if (read != sizeof(quint32)) {
        qWarning() << "[PcapIO::readUInt32] Invalid pcap file:" << field;
        return false;
    }

    if (reverseOrder)
        fourBytes = reverseBytesOrder(fourBytes);

    memcpy(temp, fourBytes.constData(), sizeof(quint32));

    return true;
}

bool PcapIO::readUInt16(quint16 *temp, QString field)
{
    QByteArray twoBytes(sizeof(quint16), '\0');
    qint64 read = file->read(twoBytes.data(), sizeof(quint16));
    if (read != sizeof(quint16)) {
        qWarning() << "[PcapIO::readUInt16] Invalid pcap file:" << field;
        return false;
    }

    if (reverseOrder)
        twoBytes = reverseBytesOrder(twoBytes);

    memcpy(temp, twoBytes.constData(), sizeof(quint16));

    return true;
}

bool PcapIO::readByteArray(QByteArray *temp, qint64 length, QString field)
{
    if (temp == nullptr) {
        qWarning() << "[PcapIO::readByteArray] null bytearray:" << field;
        return false;
    }

    temp->resize(length);

    qint64 read = file->read(temp->data(), length);
    if (read != length) {
        qWarning() << "[PcapIO::readUInt16] Invalid pcap file, bytearray:" << field;
        return false;
    }

    return true;
}

QByteArray PcapIO::packetToBytes(PcapPacket *packet)
{
    QByteArray ret;
    if (packet == nullptr) {
        qWarning() << "[PcapIO::packetToBytes] packet is nullptr";
        return ret;
    }

    // timestamp

    ret.append(uint32ToBytes(packet->getTimestampRaw()));

    // microsec

    ret.append(uint32ToBytes(packet->getMicrosec()));

    // data

    QByteArray data = packet->getData();
    if ((quint32)data.size() > maxSnapshotlenght) { // checking if the data is not too large for the current file
        qWarning() << "[PcapIO::packetToBytes] Data is larger than the max snapshot size => Truncating.";
        data = data.mid(0,maxSnapshotlenght);
        packet->setCapturedSize(maxSnapshotlenght);
    }

    // Captured size

    ret.append(uint32ToBytes(packet->getCapturedSize()));

    // original size

    ret.append(uint32ToBytes(packet->getOriginalSize()));

    // then appending the data
    ret.append(data);

    return ret;
}

QByteArray PcapIO::generatePcapHeader()
{
    QByteArray ret;

    // first is the magic number

    if (littleEndian)
        ret = PcapIO::PCAP_MAGIC_LE;
    else
        ret = PcapIO::PCAP_MAGIC_BE;

    // then the version Major
    ret.append(uint16ToBytes(versionMajor));
    // then the version Minor
    ret.append(uint16ToBytes(versionMinor));
    // then the timestamp
    ret.append(uint32ToBytes(timezoneshift));
    // precision
    ret.append(uint32ToBytes(precision));
    // maxSnapshotlenght
    ret.append(uint32ToBytes(maxSnapshotlenght));
    // linkLayerType
    ret.append(uint32ToBytes(linkLayerType));

    return ret;
}

bool PcapIO::readExistingFile()
{
    if (!file->isReadable())
        return false;

    QByteArray fourBytes(4, '\0');
    qint64 read = 0;

    // first is the magic number
    read = file->read(fourBytes.data(), 4);
    if (read != 4) {
        qWarning() << "[PcapIO::open] Invalid pcap file (too short: magic)";
        return false;
    }

    if (fourBytes == PCAP_MAGIC_LE) {
        littleEndian = true;
#ifdef Q_LITTLE_ENDIAN
        reverseOrder = false;
#else
        reverseOrder = true;
#endif
        qDebug() << "Little Endian Magic";
    } else if (fourBytes == PCAP_MAGIC_BE) {
        littleEndian = false;
#ifdef Q_LITTLE_ENDIAN
        reverseOrder = true;
#else
        reverseOrder = false;
#endif
        qDebug() << "Big Endian Magic";
    } else {
        qWarning() << "[PcapIO::open] Invalid magic number";
        return false;
    }

    qDebug() << "Magic:" << QString::fromUtf8(fourBytes.toHex());

    // then the version Major

    if (!readUInt16(&versionMajor, QString("Version Major"))) {
        return false;
    }

    // then the version Minor

    if (!readUInt16(&versionMinor, QString("Version Minor"))) {
        return false;
    }

    qDebug() << QString("version: %1.%2").arg(versionMajor).arg(versionMinor);

    // then the timestamp

    if (!readUInt32(&timezoneshift, QString("timestamp"))) {
        return false;
    }

    qDebug() << QString("timestamp: %1").arg(timezoneshift);

    // precision

    if (!readUInt32(&precision, QString("precision"))) {
        return false;
    }

    qDebug() << QString("precision: %1").arg(precision);

    // maxSnapshotlenght

    if (!readUInt32(&maxSnapshotlenght, QString("maxSnapshotlenght"))) {
        return false;
    }

    qDebug() << QString("maxSnapshotlenght: %1").arg(maxSnapshotlenght);

    // linkLayerType

    quint32 temp = 0;
    if (!readUInt32(&temp, QString("linkLayerType"))) {
        return false;
    }

    if (!PcapDef::LINK_TYPES.contains(temp)) {
        qWarning() << "[PcapIO::readExistingFile] Unknown layer type:" << temp;
        return false;
    }

    linkLayerType = (PcapDef::Link_Type)temp;

    qDebug() << QString("Link Layer: %1").arg(PcapDef::LINK_TYPES.value(linkLayerType));

    packetsStart = file->pos();
    currentPos = packetsStart;

    return true;
}


QByteArray PcapIO::uint32ToBytes(quint32 val)
{
    QByteArray fourBytes(sizeof(quint32),'\0');
    memcpy(fourBytes.data(),&val,sizeof(quint32));

    if (reverseOrder) {
        fourBytes = reverseBytesOrder(fourBytes);
    }

    return fourBytes;
}

QByteArray PcapIO::uint16ToBytes(quint16 val)
{
    QByteArray twoBytes(sizeof(quint16),'\0');
    memcpy(twoBytes.data(),&val,sizeof(quint16));

    if (reverseOrder) {
        twoBytes = reverseBytesOrder(twoBytes);
    }

    return twoBytes;
}

bool PcapIO::getLittleEndian() const
{
    return littleEndian;
}

void PcapIO::setLittleEndian(bool value)
{
    littleEndian = value;
}

QString PcapIO::getFilename() const
{
    return filename;
}

void PcapIO::setFilename(const QString &value)
{
    filename = value;
}

void PcapIO::setLinkLayerType(const PcapDef::Link_Type &value)
{
    linkLayerType = value;
}

void PcapIO::setMaxSnapshotlenght(const quint32 &value)
{
    maxSnapshotlenght = value;
}

void PcapIO::setPrecision(const quint32 &value)
{
    precision = value;
}

void PcapIO::setTimeZoneShift(const quint32 &value)
{
    timezoneshift = value;
}

void PcapIO::setVersionMinor(const quint16 &value)
{
    versionMinor = value;
}

void PcapIO::setVersionMajor(const quint16 &value)
{
    versionMajor = value;
}

PcapDef::Link_Type PcapIO::getLinkLayerType() const
{
    return linkLayerType;
}

QString PcapIO::getLinkLayerTypeString() const
{
    return PcapDef::LINK_TYPES.value(linkLayerType);
}

quint32 PcapIO::getMaxSnapshotlenght() const
{
    return maxSnapshotlenght;
}

quint32 PcapIO::getPrecision() const
{
    return precision;
}

quint32 PcapIO::getTimeZoneShift() const
{
    return timezoneshift;
}

quint16 PcapIO::getVersionMinor() const
{
    return versionMinor;
}

quint16 PcapIO::getVersionMajor() const
{
    return versionMajor;
}


