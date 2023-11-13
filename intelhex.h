/**************************************************************************************************
---------------------------------------------------------------------------------------------------
	Copyright (c) 2018
---------------------------------------------------------------------------------------------------
   File name    : intelhex.h
   Created      : 5 April 2021
   Author(s)    : Daniel Enrique Zamora Sifredo
---------------------------------------------------------------------------------------------------
**************************************************************************************************/
#ifndef INTELHEX_H
#define INTELHEX_H

#include <QObject>
#include <QFile>
#include <QList>
#include <QByteArray>
#include <QtCore/QtGlobal>

#if (!defined(QT_STATIC) && !defined(INTELHEX_STATIC))
#	ifdef QT_BUILD_ZEROCONF_LIB
#		define Q_INTELHEX_EXPORT Q_DECL_EXPORT
#	else
#		define Q_INTELHEX_EXPORT Q_DECL_IMPORT
#	endif
#else
#	define Q_INTELHEX_EXPORT
#endif

class Q_INTELHEX_EXPORT IntelHexSegment
{
public:
	IntelHexSegment() {;}
	quint32    startAdress;
	quint32    totalPages;
	QByteArray data;
};

class Q_INTELHEX_EXPORT IntelHex : public QObject
{
	Q_OBJECT
public:
	explicit IntelHex(QObject *parent = 0);
	bool open(QString fileName, quint32 pageSize);
	inline void reReadAll() { segmentIdx = 0; segmentPos = 0; }
	inline void reReadSegment() { segmentPos = 0; }
	inline quint32 totalSegments() { return segments.size(); }
	inline quint32 segmentSize() { return segments.at(segmentIdx).data.size(); }
	inline quint32 segmentPages() { return segments.at(segmentIdx).totalPages; }
	inline qint32 selectNextSegment() { return selectSegment(segmentIdx + 1); }
	qint32 selectSegment(qint32 segment);
    qint32 readPage(quint32 &currentAddress, char **data, bool readAllSegments = 1);
	inline qint32 totalPages() { return pagesInFile; }

private:
	void processHexLine(QString &line);
	quint32 getLineType(QString line);
	quint32 getLength(QString line);
	quint32 getAddress(QString line);
	quint32 getSegmentAddress(QString &line);
	quint32 getLinearAddress(QString &line);
	void checkForSkippedAddresses(quint32 address);
	QList<IntelHexSegment> segments;
	QFile file;
	quint32 linearAddress, segmentAddress, filledAddress;
	quint32 pageSize, pagesInFile;
	qint32 segmentIdx, segmentPos;
	bool oneShot;

signals:

public slots:
};

#endif // INTELHEX_H
