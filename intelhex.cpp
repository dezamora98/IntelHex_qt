/**************************************************************************************************
---------------------------------------------------------------------------------------------------
	Copyright (c) 2018
---------------------------------------------------------------------------------------------------
   File name    : intelhex.h
   Created      : 5 April 2021
   Author(s)    : Daniel Enrique Zamora Sifredo
---------------------------------------------------------------------------------------------------
**************************************************************************************************/
#include <QTextStream>
#include "intelhex.h"
#include <QDebug>


IntelHex::IntelHex(QObject *parent) : QObject(parent)
{

}

bool IntelHex::open(QString fileName, quint32 pageSize)
{
	linearAddress = 0;
	segmentAddress = 0;
	filledAddress = 0;
	pagesInFile = 0;
	oneShot = 0;
	IntelHex::pageSize = pageSize;
	reReadAll();
	segments.clear();
	segments.append(IntelHexSegment());

	file.setFileName(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug("Can't open file");
		return 0;
	}
	QTextStream in(&file);

	QString line = in.readLine();
	while(!line.isNull()) {
		if (line.startsWith(":"))
			processHexLine(line);
		line = in.readLine();
	}
	file.close();

	QList<IntelHexSegment>::iterator i;
    for (auto i:segments)
    {
        i.totalPages = static_cast<uint>(i.data.size()) / pageSize;
        pagesInFile+= i.totalPages;
	}

	return 1;
}

void IntelHex::processHexLine(QString &line)
{
	quint32 length, address, i;
	bool ok;

	switch (getLineType(line)) {
		case 4: linearAddress = getLinearAddress(line); break;
		case 2: segmentAddress = getSegmentAddress(line); break;
		case 5:
		case 1:
			// pad the very last page to be on a page boundary
            while (static_cast<uint>(segments.last().data.size()) % pageSize)
                segments.last().data.append(static_cast<char>(0xFF));
			break;
		case 0:
			length = getLength(line);
			address = linearAddress + segmentAddress + getAddress(line);
			if (!oneShot) { // initialize on first line
				oneShot = 1;
				filledAddress = address;
				segments.last().startAdress = address;
			}
			checkForSkippedAddresses(address);
			filledAddress+= length;
			i = 9;
			while (length) {
				QString temp;
				temp = line;
                temp.remove(0, static_cast<int>(i));
				temp.remove(2, 64);
				segments.last().data.append(temp.toInt(&ok, 16));
				i+= 2;
				length--;
				address++;
			}
            qDebug() << QString::number(address, 16) << line;
			break;
	}
}

quint32 IntelHex::getLineType(QString line)
{
	line.remove(0, 7);
	line.remove(2, 64);
	return line.toUInt();
}

quint32 IntelHex::getLength(QString line)
{
	bool ok;

	line.remove(0, 1);
	line.remove(2, 64);
	return line.toInt(&ok, 16);
}

quint32 IntelHex::getAddress(QString line)
{
	bool ok;

	line.remove(0, 3);
	line.remove(4, 64);
	return line.toInt(&ok, 16);
}

quint32 IntelHex::getSegmentAddress(QString &line)
{
	bool ok;

	line.remove(0, 9);
	line.remove(4, 64);
	return line.toInt(&ok, 16);
}

quint32 IntelHex::getLinearAddress(QString &line)
{
	return getSegmentAddress(line) * 0x10000;
}

void IntelHex::checkForSkippedAddresses(quint32 address)
{
	if (address != filledAddress) {
		quint32 nextPageAddress = filledAddress + (pageSize - 1);
		nextPageAddress&= ~(pageSize - 1);

		// Fill up to address or nextPageAddress, which ever comes first.
		for (;(filledAddress < nextPageAddress) && (filledAddress < address); filledAddress++)
			segments.last().data.append(0xFF);

		// if address is not page aligned, set nextPageAddress to the preceeding page boundary of address.
		nextPageAddress = address & ~(pageSize - 1);

		// check for a jump in pages
		if ((address - filledAddress) >= pageSize) {
			segments.append(IntelHexSegment());
			segments.last().startAdress = nextPageAddress;
			filledAddress = nextPageAddress;
		}

		// check if address is page aligned and fill if not
		for (;filledAddress < address; filledAddress++)
			segments.last().data.append(0xFF);
	}
}

qint32 IntelHex::selectSegment(qint32 segment)
{
	if (segment < segments.size() - 1) {
		segmentIdx = segment;
		segmentPos = 0;
		return segmentIdx;
	}
	else
		return -1;
}

//quint32 getSegmentBaseAddress()

qint32 IntelHex::readPage(quint32 &currentAddress, char **data, bool readAllSegments)
{
	qint32 bytesAvailable = segments.at(segmentIdx).data.size() - segmentPos;

	if (!bytesAvailable && readAllSegments) {
		if (segmentIdx < segments.size() - 1) {
			segmentIdx++;
			segmentPos = 0;
			bytesAvailable = segments.at(segmentIdx).data.size() - segmentPos;
		}
	}

	currentAddress = segments.at(segmentIdx).startAdress + segmentPos;
    *data = (char*)segments.at(segmentIdx).data.data() + segmentPos;

	if (bytesAvailable > (signed)pageSize)
		bytesAvailable = pageSize;
	segmentPos+= bytesAvailable;

    return bytesAvailable;
}
